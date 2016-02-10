#include <avr/io.h>
#include "MatrixKeyboard.h"


// ���� ���� ������� ������������������� ���������������� ����.
// ��������������, ��� �� ��� ���������������� ���� ������� ��
// ��������. ������ �������� � ��� � �� ������ ������ �������.
// ��� �������, ��� ������� ���������.


// ��� ��� ������� ��������� �� �����, ������������ ��� �����
// (�������� A, B � �.�.) ������������ ��� ����� ���� PORTA.
#define PRECOMP2_PORT(x)		PORT ## x
#define PRECOMP_PORT(x)			PRECOMP2_PORT(x)

// ��� ��� ������� ��������� �� �����, ������������ ��� �����
// (�������� A, B � �.�.) ������������ ��� ��������, ������������
// ������������ ������ ����� ���� DDRA.
#define PRECOMP2_DDR(x)			DDR ## x
#define PRECOMP_DDR(x)			PRECOMP2_DDR(x)

// ��� ��� ������� ��������� �� �����, ������������ ��� �����
// (�������� A, B � �.�.) ������������ ��� ��������, �� ��������
// ����� ����������� �������� ������� �����, �������� ��� PINA.
#define PRECOMP2_PIN(x)			PIN ## x
#define PRECOMP_PIN(x)			PRECOMP2_PIN(x)


// ���������� ����� ��������� ��� ������������ �����.
#define KBI_READ_PORT			PRECOMP_PORT(KB_READ_PORT)
#define KBI_READ_DDR			PRECOMP_DDR(KB_READ_PORT)
#define KBI_READ_PIN			PRECOMP_PIN(KB_READ_PORT)

// ���������� ����� ��������� ��� ������������ �����.
#define KBI_SCAN_PORT			PRECOMP_PORT(KB_SCAN_PORT)
#define KBI_SCAN_DDR			PRECOMP_DDR(KB_SCAN_PORT)


// ��� ���� ��������� ������ ������������ ������� ����� ���
// ������������ �����. �����������, ��� ����� KB_SCAN_LINES_MASK
// ����� 0b11110000. ���� ������ ���������� �� ��� ������ ��
// ��������� ��������:
//    0b11100000
//    0b11010000
//    0b10110000
//    0b01110000
// ���� �����, ���� ��� ����������� ���������, ��� ���������
// ������ �� ������� ����������� ������������.
unsigned char KBI_SCAN_LINES_MASKS[KB_SCAN_LINES_COUNT] = {

#if KB_SCAN_LINES_MASK & 0x01
	KB_SCAN_LINES_MASK - 0x01,
#endif

#if KB_SCAN_LINES_MASK & 0x02
	KB_SCAN_LINES_MASK - 0x02,
#endif

#if KB_SCAN_LINES_MASK & 0x04
	KB_SCAN_LINES_MASK - 0x04,
#endif

#if KB_SCAN_LINES_MASK & 0x08
	KB_SCAN_LINES_MASK - 0x08,
#endif

#if KB_SCAN_LINES_MASK & 0x10
	KB_SCAN_LINES_MASK - 0x10,
#endif

#if KB_SCAN_LINES_MASK & 0x20
	KB_SCAN_LINES_MASK - 0x20,
#endif

#if KB_SCAN_LINES_MASK & 0x40
	KB_SCAN_LINES_MASK - 0x40,
#endif

#if KB_SCAN_LINES_MASK & 0x80
	KB_SCAN_LINES_MASK - 0x80,
#endif

};



// ���. �������� ��������. ������ ��������� �������� �������,
// ���� ������� �� ����� ����� �����������.

// ��� ������� ��������� ������������� ��������� ����������.
void InitializeKeyboard()
{
	// ����������� ����� ������������� ��� ����.
	KBI_READ_DDR &= ~KB_READ_LINES_MASK;

	// ����������� ����� ������������� ��� �����.
	KBI_SCAN_DDR |= KB_SCAN_LINES_MASK;

	// ���������� ������������� ��������� � ����������� ������.
	KBI_READ_PORT |= KB_READ_LINES_MASK;
}


// ��� ������� ���������� ���������� � ���������� ����� �������
// ������. �� ���� ���� � ������������ �������� ���������� ��� 0,
// ������ ���� ������ ������� � ������� 0. � ��� �����.
KEYS ReadFromKeyboard()
{
	KEYS result = 0;

	for (int i = 0; i < KB_SCAN_LINES_COUNT; i++)
	{
		KBI_SCAN_PORT &= ~(KB_SCAN_LINES_MASK);
		KBI_SCAN_PORT |= KBI_SCAN_LINES_MASKS[i];

		unsigned char tmpResult = (unsigned char)(~(KBI_READ_PIN & KB_READ_LINES_MASK) & KB_READ_LINES_MASK);
		result |= (KEYS)tmpResult << (KB_READ_LINES_COUNT * i);
	}

	return result;
}
