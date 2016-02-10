#ifndef _MATRIXKEYBOARD_H_
#define _MATRIXKEYBOARD_H_

// ��� ������������ �����. ��������, ��������� ������ �����,
// �������� A, B  � ���� ��������. �� ����� ����� ����� �����
// ������������ ����� ����������� ���������.
#define KB_READ_PORT	F

// ���������� ����������� �����.
#define KB_READ_LINES_COUNT	(4)

// ����� ����������� �����. ����, ������������� � 1, ��������� �� ��,
// ��� ��������������� ����� ����� �������� �����������. ��������,
// ���� ���������� ������ ���, ������ ������ ����� ����� KB_READ_PORT
// �������� �����������. � ��� �����. � ���� ������������ ����� �����
// � 0 �� 3 ������������. ���� KB_READ_PORT � KB_SCAN_PORT ���������,
// ���� KB_READ_LINES_MASK � KB_SCAN_LINES_MASKS �� ������ ������������.
#define KB_READ_LINES_MASK	0b00001111

// ��� ������������ �����. ��������, ��������� ������ �����,
// �������� A, B  � ���� ��������. �� ����� ����� ����� �����
// ������������ ����� ����������� ���������.
#define KB_SCAN_PORT	F

// ���������� ����������� �����.
#define KB_SCAN_LINES_COUNT	(4)

// ����� ����������� �����. ����, ������������� � 1, ��������� �� ��,
// ��� ��������������� ����� ����� �������� �����������. ��������,
// ���� ���������� ������ ���, ������ ������ ����� ����� KB_SCAN_PORT
// �������� �����������. � ��� �����. � ���� ������������ ����� �����
// � 4 �� 7 ������������. ���� KB_READ_PORT � KB_SCAN_PORT ���������,
// ���� KB_READ_LINES_MASK � KB_SCAN_LINES_MASKS �� ������ ������������.
#define KB_SCAN_LINES_MASK	0b11110000


// ���� ������ ��������� ������� ����������� ��� ������, ������������
// �������� ReadFromKeyboard. ��� ���������� ��� �����������.
#if (KB_READ_LINES_COUNT * KB_SCAN_LINES_COUNT) <= 8
typedef unsigned char KEYS;
#elif (KB_READ_LINES_COUNT * KB_SCAN_LINES_COUNT) <= 16
typedef unsigned short KEYS;
#elif (KB_READ_LINES_COUNT * KB_SCAN_LINES_COUNT) <= 32
typedef unsigned long KEYS;
#elif (KB_READ_LINES_COUNT * KB_SCAN_LINES_COUNT) <= 64
typedef unsigned long long KEYS;
#else
// ���� ��� �� ������������ ����� 64 ������, �� �� ������ ���
// ��������������.
#error "Wow! This source code not supported more that 64 keys."
#endif


// ��� ������� ��������� ������������� ��������� ����������.
void InitializeKeyboard();

// ��� ������� ���������� ���������� � ���������� ����� �������
// ������. �� ���� ���� � ������������ �������� ���������� ��� 0,
// ������ ���� ������ ������� � ������� 0. � ��� �����.
KEYS ReadFromKeyboard();

#endif // _MATRIXKEYBOARD_H_
