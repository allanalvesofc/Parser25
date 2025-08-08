#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "shlwapi.lib")

// Blacklist chars
static const wchar_t blacklist[] = L"'`´\"%{}[]^~#&$\\";

// Checa se o caractere está na blacklist - versão simples
__declspec(noinline) int IsBlacklistedChar(wchar_t ch) {
	for (size_t i = 0; i < sizeof(blacklist) / sizeof(wchar_t) - 1; i++) {
		if (ch == blacklist[i]) return 1;
	}
	return 0;
}

// Remove chars blacklist de buffer inplace, retorna novo tamanho
void CleanBuffer(wchar_t* data, size_t* len) {
	wchar_t* src = data;
	wchar_t* dst = data;
	wchar_t ch;
	size_t originalLen = *len;

	for (size_t i = 0; i < originalLen; i++) {
		ch = src[i];
		if (!IsBlacklistedChar(ch) && ch != L'\n' && ch != L'\r') {
			*dst++ = ch;
		}
	}
	*len = (size_t)(dst - data);
	*dst = 0;
}


// Versão simples sem STL, retorna 1 se url contém substring
int ContainsSubstring(const wchar_t* str, const wchar_t* substr) {
	return (StrStrW(str, substr) != NULL);
}

// Verifica se a URL está em blacklist
int IsBlacklistedUrl(const wchar_t* url) {
	if (ContainsSubstring(url, L"192.168.") ||
		ContainsSubstring(url, L"127.0.0.1") ||
		ContainsSubstring(url, L"localhost") ||
		ContainsSubstring(url, L".vn/") ||
		ContainsSubstring(url, L".cz/") ||
		ContainsSubstring(url, L".uk/") ||
		ContainsSubstring(url, L".cn/") ||
		ContainsSubstring(url, L"android"))
		return 1;
	return 0;
}

// Função para substituir espaços por '/'
void ReplaceSpaces(wchar_t* str) {
	while (*str) {
		if (*str == L' ')
			*str = L'/';
		str++;
	}
}

// Função para processar cada linha
void ProcessLine(wchar_t* line, HANDLE hOut) {
	size_t len = wcslen(line);
	if (len < 3) return;

	ReplaceSpaces(line);

	if (len >= 2 && line[len - 2] == L':' && line[len - 1] == L':')
		return;

	// Remove http:// ou https://
	if (wcsncmp(line, L"http://", 7) == 0) {
		memmove(line, line + 7, (len - 6) * sizeof(wchar_t));
		len -= 7;
	}
	else if (wcsncmp(line, L"https://", 8) == 0) {
		memmove(line, line + 8, (len - 7) * sizeof(wchar_t));
		len -= 8;
	}

	// Encontrar os dois primeiros ':'
	wchar_t* first = wcschr(line, L':');
	if (!first) return;
	wchar_t* second = wcschr(first + 1, L':');
	if (!second) return;

	// Extrair url, login e senha (substrings)
	size_t urlLen = first - line;
	size_t loginLen = second - first - 1;
	size_t passLen = wcslen(second + 1);

	if (urlLen > 150 || loginLen > 100 || passLen > 100)
		return;

	// Buffers para cópia
	wchar_t url[151] = { 0 }, login[101] = { 0 }, pass[101] = { 0 };

	wcsncpy(url, line, urlLen);
	url[urlLen] = 0;
	wcsncpy(login, first + 1, loginLen);
	login[loginLen] = 0;
	wcsncpy(pass, second + 1, passLen);
	pass[passLen] = 0;

	if (IsBlacklistedUrl(url))
		return;

	CleanBuffer(url, &urlLen);
	CleanBuffer(login, &loginLen);
	CleanBuffer(pass, &passLen);

	// Criar saída no formato ('url', 'login', 'pass');\n
	wchar_t output[1024] = { 0 };
	swprintf(output, 1024, L"('%s', '%s', '%s');\n", url, login, pass);

	// WriteFile em UTF-8 corretamente
	char outputMB[2048] = { 0 };
	int bytes = WideCharToMultiByte(CP_UTF8, 0, output, -1, outputMB, sizeof(outputMB), NULL, NULL);
	if (bytes > 1) { // -1 inclui o '\0', então removemos ele da escrita
		DWORD writtenBytes;
		WriteFile(hOut, outputMB, bytes - 1, &writtenBytes, NULL);
	}
}

// Thread worker para processar cada arquivo
DWORD WINAPI ThreadProcessFile(LPVOID param) {
	wchar_t* filepath = (wchar_t*)param;

	HANDLE hFile = CreateFileW(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		free(filepath);
		return 1;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0) {
		CloseHandle(hFile);
		free(filepath);
		return 1;
	}

	HANDLE hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hMap) {
		CloseHandle(hFile);
		free(filepath);
		return 1;
	}

	LPVOID fileData = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (!fileData) {
		CloseHandle(hMap);
		CloseHandle(hFile);
		free(filepath);
		return 1;
	}

	// Abrir arquivo de saída
	wchar_t outFile[MAX_PATH];
	swprintf(outFile, MAX_PATH, L"%s.txt", filepath);

	HANDLE hOut = CreateFileW(outFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		UnmapViewOfFile(fileData);
		CloseHandle(hMap);
		CloseHandle(hFile);
		free(filepath);
		return 1;
	}

	// Agora vamos tratar arquivo como bytes e converter linha a linha
	char* data = (char*)fileData;
	size_t size = (size_t)fileSize.QuadPart;

	char* lineStart = data;
	char* end = data + size;

	while (lineStart < end && *lineStart) {
		char* lineEnd = (char*)memchr(lineStart, '\n', (size_t)(end - lineStart));
		if (!lineEnd) lineEnd = end;

		size_t lineLen = (size_t)(lineEnd - lineStart);

		if (lineLen > 0) {
			wchar_t line[512];
			int wlen = MultiByteToWideChar(CP_UTF8, 0, lineStart, (int)lineLen, line, 511);
			line[wlen] = 0;

			ProcessLine(line, hOut);
		}

		if (lineEnd == end) break;
		lineStart = lineEnd + 1;
	}

	CloseHandle(hOut);
	UnmapViewOfFile(fileData);
	CloseHandle(hMap);
	CloseHandle(hFile);
	free(filepath);

	wprintf(L"Arquivo processado: %s\n", filepath);
	return 0;
}

int wmain() {
	wchar_t fileBuffer[32768] = { 0 };
	OPENFILENAMEW ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"Todos Arquivos\0*.*\0";
	ofn.lpstrFile = fileBuffer;
	ofn.nMaxFile = sizeof(fileBuffer) / sizeof(wchar_t);
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.lpstrTitle = L"Selecione arquivos";

	if (!GetOpenFileNameW(&ofn)) {
		wprintf(L"Nenhum arquivo selecionado.\n");
		return 1;
	}

	// Parse multiple selection
	wchar_t* ptr = fileBuffer;
	size_t folderLen = wcslen(ptr);
	wchar_t** fileList = NULL;
	size_t fileCount = 0;

	if (ptr[folderLen + 1] == 0) {
		// Apenas um arquivo
		fileCount = 1;
		fileList = &ptr;
	}
	else {
		// Vários arquivos
		fileCount = 0;
		wchar_t* p = ptr + folderLen + 1;
		// contar arquivos
		while (*p) {
			fileCount++;
			p += wcslen(p) + 1;
		}
		fileList = (wchar_t**)malloc(fileCount * sizeof(wchar_t*));
		p = ptr + folderLen + 1;
		for (size_t i = 0; i < fileCount; i++) {
			fileList[i] = p;
			p += wcslen(p) + 1;
		}
	}

	// Cria threads para cada arquivo
	HANDLE* threads = (HANDLE*)malloc(fileCount * sizeof(HANDLE));
	for (size_t i = 0; i < fileCount; i++) {
		// montar caminho completo
		wchar_t fullPath[MAX_PATH];
		if (fileCount == 1) {
			wcscpy(fullPath, fileList[0]);
		}
		else {
			swprintf(fullPath, MAX_PATH, L"%s\\%s", ptr, fileList[i]);
		}
		// Aloca buffer para passar para thread
		wchar_t* pathCopy = (wchar_t*)malloc(MAX_PATH * sizeof(wchar_t));
		wcscpy(pathCopy, fullPath);

		threads[i] = CreateThread(NULL, 0, ThreadProcessFile, pathCopy, 0, NULL);
	}

	// Espera todas as threads terminarem
	WaitForMultipleObjects((DWORD)fileCount, threads, TRUE, INFINITE);

	for (size_t i = 0; i < fileCount; i++) {
		CloseHandle(threads[i]);
	}

	free(threads);
	if (fileCount > 1)
		free(fileList);

	wprintf(L"Todos arquivos processados.\n");
	return 0;
}
