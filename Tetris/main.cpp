// �Q�[���v���O���~���O�̏K��e�g���X#1 (WindowsGDI + raw level��Wave����API)
//	����:
//		�Ewindows7������Aero�𖳌������邾����Frame per second���{�ɂȂ�
#include "stdafx.h"
#include "Tetris.h"
#include "Util.h"
#include <math.h>
#include <stdarg.h>
#include <MMSystem.h>
#include <time.h>
#pragma comment(lib, "winmm.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

// ��`�ƃ}�N��
#define MAX_LOADSTRING 100
#define MAX_FPS 3000			// MAX_FPS
#define GAME_SPEED_FPS MAX_FPS	// �Q�[������FPS(tick count�Ɠǂ񂾕����K��?)
#define VIEW_SPEED_FPS MAX_FPS	// �`���FPS(��ʓI�ȈӖ��ł�FPS)
#define IF_KEYPRESS(vk) (::GetAsyncKeyState(vk) & 0x8000) // vk(virtual key)��press����Ă邩�ǂ���

#define SCORE_BLOCK_LANDING 5
#define SCORE_LINE_DELETE 10

#define ROTATE_LEFT 0
#define ROTATE_RIGHT 1

#define SOUND_DEBUG_SAMPLE L"sound.wav"
#define SOUND_BACKGROUND_MUSIC SOUND_DEBUG_SAMPLE
#define SOUND_MOVE SOUND_DEBUG_SAMPLE
#define SOUND_ROTATE SOUND_DEBUG_SAMPLE
#define SOUND_LINE_DELETE SOUND_DEBUG_SAMPLE
#define SOUND_GAMEOVER SOUND_DEBUG_SAMPLE
#define SOUND_ERROR SOUND_DEBUG_SAMPLE

// �O���[�o���ϐ� (Windows�֌W)
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��
HWND g_hWnd = NULL;
HDC g_mDC = NULL;
RECT g_windowRect;

// �g�p����t�H���g�̒�`
HFONT font_MSUIGothic = ::CreateFont(
		15,                   // �t�H���g�̍���(�傫��)�B
		0,                    // �t�H���g�̕��B���ʂO�B
		0,                    // �p�x�B�O�łn�j�B
		0,                    // �������p�x�B������O�B
		FW_DONTCARE,          // �����̑����B
		FALSE,                // �t�H���g���C�^���b�N�Ȃ�TRUE���w��B
		FALSE,                // �����������Ȃ�TRUE�B
		FALSE,                // ���������������Ȃ�TRUE�B
		SHIFTJIS_CHARSET,     // �t�H���g�̕����Z�b�g�B���̂܂܂łn�j�B
		OUT_DEFAULT_PRECIS,   // �o�͐��x�̐ݒ�B���̂܂܂łn�j�B
		CLIP_DEFAULT_PRECIS,  // �N���b�s���O���x�B���̂܂܂łn�j�B
		DRAFT_QUALITY,        // �t�H���g�̏o�͕i���B���̂܂܂łn�j�B
		DEFAULT_PITCH,        // �t�H���g�̃s�b�`�ƃt�@�~�����w��B���̂܂܂łn�j�B
		_T("MS UI Gothic") // �t�H���g�̃^�C�v�t�F�C�X���̎w��B����͌����܂�܁B
    );

// �O���[�o���ϐ� (�e�g���X�֌W)
#define FIELD_X_SIZE 10	// �e�g���X���E�̉���
#define FIELD_Y_SIZE 20 // �e�g���X���E�̏c��
int g_field[FIELD_X_SIZE][FIELD_Y_SIZE] = {0};			// �Œ艻���ꂽ���̂ɒǉ����āA�v���C���[��"�����Ă�"�u���b�N�̍��W���܂܂ꂽ�z��
int g_puted_field[FIELD_X_SIZE][FIELD_Y_SIZE] = {0};	// ���ۂɌŒ艻���ꂽ�u���b�N�̍��W�i�[�p�z��
int g_focusX = FIELD_X_SIZE / 2;	// ���ݒl(X)
int g_focusY = 0;					// ���ݒl(Y)
int g_degree_index = 0; // 0 - TETRIS_R_MAX
DWORD g_frameRate = 0;	// �t���[�����[�g�v���p
DWORD g_frame = 0;		// �t���[�����v���p
int g_tetris_type = 0;	// �e�g���X���(I�Ƃ�O�Ƃ�)
int g_status = 0;		// �Q�[���̏��(��ԑJ�ڂɗ��p)
int g_score = 0;		// �X�R�A���v
int g_time = 0;			// �o�ߕb��
bool bgm_enabled = false;	// BGM ON/OFF
#define TETRIS_TYPE_MAX 7	// �e�g���X�̎�ޓY������
#define TETRIS_R_MAX 4		// �e�g���X�̊p�x�Y������
#define TETRIS_X_SIZE 4		// �e�g���X��X�Y������
#define TETRIS_Y_SIZE 4		// �e�g���X��Y�Y������
// �e�g���X�̃f�[�^�z��(���̓Y������A�e�g���X��ʁA�p�x(0,90,180,270)�Ax�Ay)
int blocks[TETRIS_TYPE_MAX][TETRIS_R_MAX][TETRIS_X_SIZE][TETRIS_Y_SIZE] = {
	{ // I
		{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
		{{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
		{{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
		{{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}
	},
	{ // J
		{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
		{{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
		{{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
		{{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}}
	},
	{ // L
		{{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
		{{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
		{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
		{{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}
	},
	{ // S
		{{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}},
		{{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
		{{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
		{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
	},
	{ // Z
		{{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
		{{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
		{{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
		{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}
	},
	{ // T
		{{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
		{{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
		{{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
		{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}
	},
	{ // O
		{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
		{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
		{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
		{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}
	}
};

// �O���[�o���ϐ��i�e�g���X�֌W�A�f�o�b�O�p)

// �֐���`
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// �w�肳�ꂽ���W�ɁA�w�肳�ꂽ��ʁA�p�x�̃e�g���X���Z�b�g���邱�Ƃ��o���邩�ǂ������肷�邽�߂̊֐�
bool hitTestTetrisOverlap(int type, int r, int xx, int yy)
{
	for(int y=0; y<TETRIS_Y_SIZE; y++){
		for(int x=0; x<TETRIS_X_SIZE; x++){
			// �e���v���[�g�̃u���b�N�͂����ɂ܂�����??
			if( blocks[type][r][x][y] == 1 ){
				// ���݂̃A�N�e�B�u�Ȉʒu�ɕϊ�
				int active_x = x + xx;
				int active_y = y + yy;

				// ���̈ʒu�͉�ʓ��Ɏ��܂��Ă�H�H
				// ���܂��ĂȂ��Ƃ��͎��܂��ĂȂ��������ƂɌ��݈ʒu��␳���Ă�����
				if(active_x < 0){ // ������
					return true;
				}

				if(FIELD_X_SIZE - 1 < active_x){ // �E����
					return true;
				}

				if(FIELD_Y_SIZE - 1 < active_y){ // ������
					return true;
				}

				// �ړ��悪���̃u���b�N�Əd�Ȃ��Ă邩����
				if(g_puted_field[active_x][active_y] == 1){
					return true;
				}
			}
		}
	}
	return false;
}

void onPaint(HWND hWnd, HDC hdc)
{
	// �w�i�S���h��Ԃ�
	FillRectBrush(hdc, g_windowRect.left, g_windowRect.top,
		g_windowRect.right - g_windowRect.left,
		g_windowRect.bottom - g_windowRect.top, RGB(0,0,0));

	// �e�g���~�m�̓񎟌��z���`�悷��
	int block_width;
	int block_height;
	block_width = block_height = 20;
	COLORREF block_color = RGB(255,0,0);
	COLORREF fixed_block_color = RGB(0,255,0);
	COLORREF background_color = RGB(255,255,255);
	
	int padding_top = 30;
	int padding_left = 20;
	for(int y=0; y<FIELD_Y_SIZE; y++){
		for(int x=0; x<FIELD_X_SIZE; x++){
			if(g_puted_field[x][y] == 1){ // �z�u�ς݃e�g���X�̕`��
				BorderedRect(hdc, padding_left + x * block_width, padding_top + y * block_height, block_width, block_height, fixed_block_color);
			}else if(g_field[x][y] == 1){ // �A�N�e�B�u�ȓ����Ă�e�g���X�̕`��
				BorderedRect(hdc, padding_left + x * block_width, padding_top + y * block_height, block_width, block_height, block_color);
			}
		}
	}

	// �X�R�A�\�������̕`��
	// �w�i�̕`��
	int scoreboard_x = FIELD_X_SIZE * block_width + padding_left + 20;
	int scoreboard_y = padding_top;
	FillRectBrush(hdc, scoreboard_x, scoreboard_y, 200, 20, RGB(255,255,255));
	
	RECT rect;
	rect.left = scoreboard_x;
	rect.top = scoreboard_y;
	rect.right = rect.left + 200;
	rect.bottom = rect.top + 20;
	::DrawFormatText(hdc, &rect, DT_RIGHT, L"%d", g_score);
	::DrawFormatText(hdc, &rect, DT_LEFT, L"�X�R�A:");
	
	// �o�ߎ��Ԃ̕\��
	FillRectBrush(hdc, scoreboard_x, scoreboard_y + 20 + 20, 200, 20, RGB(255, 255, 255));
	rect.left = scoreboard_x;
	rect.top = scoreboard_y + 20 + 20;
	rect.right = rect.left + 200;
	rect.bottom = rect.top + 20;
	::DrawFormatText(hdc, &rect, DT_RIGHT, L"%d", g_time);
	::DrawFormatText(hdc, &rect, DT_LEFT, L"�o�ߎ���(�b):");

	// FPS�̕`��(�f�o�b�O�I�[�o�[���C)
	HFONT oldFont = (HFONT)::SelectObject(hdc, font_MSUIGothic);
	::TextFormatOut(hdc, 0, 0, L"FPS: %d/s (%df)", g_frameRate, g_frame);
	::SelectObject(hdc, oldFont);
	::DeleteObject(font_MSUIGothic);
	
	// �O�g�`��
	drawRectColor(hdc, padding_left, padding_top, FIELD_X_SIZE * block_width, FIELD_Y_SIZE * block_width, RGB(0,255,0), 1);
}

void rotateTetrisIndex(int index)
{
	g_degree_index = index;
	trace(L"g_degree_index: %d\n", g_degree_index);
}

void rotateTetris(void)
{
	if(++g_degree_index > TETRIS_R_MAX - 1){
		g_degree_index = 0;
	}
}

void reverseRotateTetris(void)
{
	if(--g_degree_index < 0)
		g_degree_index = TETRIS_R_MAX - 1;
}

void flipTetris()
{
	if(++g_tetris_type >= TETRIS_TYPE_MAX)
		g_tetris_type = 0;

	// �e�g���X��ύX�����Ƃ��́A�p�x�����ɖ߂�
	rotateTetrisIndex(0);
}

// ���݂̐ݒ��N�b������̃t���[������
// �`���FPS�ł͂Ȃ��āA�Q�[�������Ԃ̕�
int getFramesOfSeconds(int seconds)
{
	return seconds * GAME_SPEED_FPS;
}

// �����_���Ƀe�g���X��ύX���܂�
void randomChangeTetris()
{
	g_tetris_type = rand() % TETRIS_TYPE_MAX; // 0-6�������_���ɕԋp
	rotateTetrisIndex(rand() % TETRIS_R_MAX); // �p�x��0-3��4����
}

void GameMain_main(HWND hWnd)
{
	// 1�b�Ɉ��A�v���C���[�̃u���b�N������Ɉړ�������
	static int timeCount = 0;	// FPS���Œ艻�����邽�߂Ɏg�p���܂�
	if(timeCount++ > getFramesOfSeconds(1)){
		timeCount = 0;
		g_focusY++;
		g_time += 1; // �o�ߎ��Ԃ�1sec���Z���܂�
		::PlaySound(SOUND_MOVE, NULL, SND_FILENAME | SND_ASYNC);
	}
	
	// �t�B�[���h�̏�����
	for(int y=0; y<FIELD_Y_SIZE; y++){
		for(int x=0; x<FIELD_X_SIZE; x++){
			g_field[x][y] = 0;
		}
	}
	
	// ���݃A�N�e�B�u�ȃe�g���~�m�̕`��
	// �t�H�[�J�X���Ă�ꏊ�����_�Ƃ��āA���ݑI�𒆂̃e�g���X��`�悷��
	for(int y=0; y<TETRIS_Y_SIZE; y++){
		for(int x=0; x<TETRIS_X_SIZE; x++){
			if( blocks[g_tetris_type][g_degree_index][x][y] == 1){
				// �Q�[���̗L���͈͓�����ˁH
				int active_x = x + g_focusX;
				int active_y = y + g_focusY;
				g_field[active_x][active_y] = 1;
			}
		}
	}

	// ���݃A�N�e�B�u�ȃe�g���~�m�̒��n����
	// �e�g���~�m���A�����������̃u���b�N�����邱�Ƃ����o�����A���邢�͒n�ʂ����邱�Ƃ����o�����ꍇ
	bool landing_flag = false;
	for(int x=0; x<TETRIS_X_SIZE; x++){
		for(int y=TETRIS_Y_SIZE-1; y>=0; y--){
			bool block_found = false; // ����Y���Ƀu���b�N�����邩�ǂ�����\��

			// ��ԍŏ���1(����)����u���b�N���������Ƃ�
			// ���̉��Ƀu���b�N���ݒu����Ă�����ݒu�t���O��ON�ɂ���
			if( block_found == false && blocks[g_tetris_type][g_degree_index][x][y] == 1 ){
				// �u���b�N������Y���ł��łɈ�x�ł��������Ă���ꍇ�͂��̏������s��Ȃ�
				block_found = true;

				// ��΍��W�ɕϊ����Ď��������ЂƂ��Ƀu���b�N���邩����
				if( g_puted_field[g_focusX + x][g_focusY + y + 1] == 1 ){
					landing_flag = true;
				}

				// ���邢�́AY���̍ŉ��s�ɓ��B���Ă��邩
				if( g_focusY + y >= FIELD_Y_SIZE - 1 ){
					landing_flag = true;
				}
			}
		}
	}

	// ���n���Ă��ꍇ�͌��݂̏ꏊ�ɌŒ艻
	if(landing_flag){
		for(int y=0; y<TETRIS_Y_SIZE; y++){
			for(int x=0; x<TETRIS_X_SIZE; x++){
				if(blocks[g_tetris_type][g_degree_index][x][y] == 1){
					g_puted_field[g_focusX + x][g_focusY + y] = 1;
				}
			}
		}

		// �X�R�A���Z
		g_score += SCORE_BLOCK_LANDING;

		// ���̃e�g���~�m�������_���ɑI��
		randomChangeTetris();

		// �����ʒu�ɔz�u����
		g_focusX = (FIELD_X_SIZE - 1) / 2;
		g_focusY = 0;
	}

	// �e�g���~�m�̏�������
	// �����fill����Ă���A���̍s���폜����
	// ���ׂẴu���b�N������Ɉړ����܂�
	bool line_delete_flag = false;
	for(int y=0; y<FIELD_Y_SIZE; y++){
		bool line_fill_flag = true;
		for(int x=0; x<FIELD_X_SIZE; x++){
			if(g_puted_field[x][y] == 0){
				line_fill_flag = false; // �s���Ɉ�ł������������ꂽ��Ȍ�x���̃��[�v�͂��������Ȃ��̂Ŕ����܂�
				break;
			}
		}

		// 1�s���ׂău���b�N���z�u����Ă����s(y)�������
		// ���̍s�̃u���b�N���폜���āA���������s�̃u���b�N��S�Ĉ���Ɉړ�������
		if(line_fill_flag == true){
			trace(L"detect fill line: %d(index) (all line: %dgyou)", y, FIELD_Y_SIZE);
			for(int x=0; x<FIELD_X_SIZE; x++){
				g_puted_field[x][y] = 0;
			}
			trace(L"clear line");

			// ���̍s������̃u���b�N�����ׂĈ���ɉ����鏈��
			trace(L"move upper lines");
			for(int yy=y; yy>0; yy--){
				for(int xx=0; xx<FIELD_X_SIZE; xx++){
					g_puted_field[xx][yy] = g_puted_field[xx][yy-1];
				}
			}

			// �X�R�A���Z
			g_score += SCORE_LINE_DELETE;

			// �s���폜��������
			line_delete_flag = true;
		}
	}

	// ��s�ł��s���폜�����Ƃ��͐�p�̉��𗬂�
	if(line_delete_flag == true){
		::PlaySound(SOUND_LINE_DELETE, NULL, SND_FILENAME | SND_ASYNC);
	}

	// �Q�[���I�[�o�[����
	// ��ԏ�̍s�Ƀu���b�N���Œ艻����Ă����ꍇ�A�Q�[���I�[�o�[��ԂɈڍs���܂�
	for(int x=0; x<FIELD_X_SIZE; x++){
		if( g_puted_field[x][0] == 1 ){
			g_status = 1;
		}
	}
}

void Game_init(){
	// �����̏�����
	srand((unsigned int)time(0));
	randomChangeTetris();

	if(bgm_enabled){
		::mciPlayBGM(SOUND_BACKGROUND_MUSIC, 0.1);
	}
}

void GameMain_gameOver(HWND hWnd){
	::PlaySound(SOUND_GAMEOVER, NULL, SND_FILENAME);
}

void GameMain_quit(){
	trace(L"�t�H���g�̉��\n");
	::DeleteObject(font_MSUIGothic);
}

// false��ԋp����ƏI�������Ɉڍs����
bool GameMain(HWND hWnd)
{
	switch(g_status){
	case 0:
		GameMain_main(hWnd);
		break;
	case 1:
		GameMain_gameOver(hWnd);
		return false;
		break;
	default:
		trace(L"UNDEFINED TYPE: %d\n", g_status);
	}

	return true;
}

// �e�g���X����]�����Ă����v�Ȏ�������]������
// direct�͕����A1���E�A����ȊO����
void rotateTetrisSafety(int type, int x, int y, int direct)
{
	// ��]������O�ɁA��]�����Ă����v���̊m�F����������
	// ���肵����]��̍��W�A�p�x�A��ނ̃e�g���X�ɑ΂���
	// ���݂̃��[���h�œ����蔻��s��
	int backup_degree = g_degree_index;

	if(direct == 1)
		rotateTetris();
	else
		reverseRotateTetris();

	int r = g_degree_index;
	rotateTetrisIndex(backup_degree);

	// �ړ��悪���̃u���b�N�Əd�Ȃ��ĂȂ������Ƃ��݈̂ړ�����������
	// ���̏�ł��̏ꏊ����ʊO��������␳���Ă�����
	if(::hitTestTetrisOverlap(g_tetris_type, r, g_focusX, g_focusY) == true){
		::PlaySound(SOUND_ERROR, NULL, SND_FILENAME | SND_ASYNC);
	}else{
		rotateTetris(); // ���ۂɉ�]�����܂�
		::PlaySound(SOUND_ROTATE, NULL, SND_FILENAME | SND_ASYNC);
	}
}

void onKeyDown(DWORD key)
{
	switch(key){
	case 'F':
		flipTetris(); // �e�g���X�̎�ޕύX
		break;
	case 'E': // �E��]
		rotateTetrisSafety(g_tetris_type, g_focusX, g_focusY, ROTATE_RIGHT); // ����]
		break;
	case 'Q':
		rotateTetrisSafety(g_tetris_type, g_focusX, g_focusY, ROTATE_LEFT); // ����]
		break;
	case 'R':
		rotateTetrisSafety(g_tetris_type, g_focusX, g_focusY, ROTATE_RIGHT); // �E��]
		break;
	case 'Z': // �����I��
		g_status = 1;
		break;
	default:
		// �v���C���[�̓��͒l����ɏ���
		bool move_key_pressed = false;

		// ���݂̍��W��ۑ�
		int cur_x = g_focusX;
		int cur_y = g_focusY;
		if(IF_KEYPRESS('A') || IF_KEYPRESS('H') || IF_KEYPRESS(VK_LEFT)){
			cur_x--;
			move_key_pressed = true;
		}
		if(IF_KEYPRESS('S') || IF_KEYPRESS('J') || IF_KEYPRESS(VK_DOWN)){
			cur_y++;
			move_key_pressed = true;
		}
		if(IF_KEYPRESS('D') || IF_KEYPRESS('L') || IF_KEYPRESS(VK_RIGHT)){
			cur_x++;
			move_key_pressed = true;
		}
		
		// �w�肳�ꂽ���W�A�p�x�A��ނ̃e�g���X�ɑ΂���
		// ���݂̃��[���h�Ńq�b�g�e�X�g���s��
		if(move_key_pressed == true){
			// �ړ��悪���̃u���b�N�Əd�Ȃ��ĂȂ������Ƃ��݈̂ړ�����������
			// ���̏�ł��̏ꏊ����ʊO��������␳���Ă�����
			if(::hitTestTetrisOverlap(g_tetris_type, g_degree_index, cur_x, cur_y) == false){ // �d�Ȃ��ĂȂ������Ƃ�
				g_focusX = cur_x; // �ړ��𐬗�������
				g_focusY = cur_y;

				// �ړ������������Ƃ��̂݉����o��
				::PlaySound(SOUND_MOVE, NULL, SND_FILENAME | SND_ASYNC);
			}
		}
		break;
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// �O���[�o������������������Ă��܂��B
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TETRIS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TETRIS));

	// ���C�� ���b�Z�[�W ���[�v:
	::timeBeginPeriod(1);
	
	double host_timescale = 1;
	double view_max_fps = VIEW_SPEED_FPS; // �`���FPS����
	// ��̂�������net_graph��FPS���Ǝv��
	double game_max_fps = GAME_SPEED_FPS * host_timescale; // �Q�[���X�s�[�h��FPS����
	// ����������ʓI�ɂ�tick??

	DWORD dwTime = ::timeGetTime();
	DWORD fps_after = dwTime;
	DWORD fps_before = fps_after;
	DWORD frame = 0;
	DWORD currentTime = dwTime;
	double gameSpeedWaitTime = (1.0 / game_max_fps * 1000.0);
	double gameSpeedNextTime = dwTime + gameSpeedWaitTime;
	
	double viewSpeedWaitTime = (1.0 / view_max_fps * 1000.0);
	double viewSpeedNextTime = dwTime + gameSpeedWaitTime;

	Game_init();

	while(true){
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)){
			if(!GetMessage(&msg, NULL, 0, 0))
				return msg.wParam;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}else{
			// FPS����(1000ms�b�������frame�����v��)
			currentTime = ::timeGetTime();
			if(currentTime - fps_before >= 1000){ // �O�񂩂�1�b��(1000ms)�o�߂��Ă����ꍇ
				fps_before = currentTime;
				g_frameRate = g_frame;
				g_frame = 0;	// �t���[���������b�̌v���̂��߂Ƀ��Z�b�g
			}
	
			// FPS����(1/60�����ɃR�[�h�����s�����悤�ɂ���)
			if(currentTime >= gameSpeedNextTime ){ // ����̗\����s�����߂��Ă����ꍇ
				gameSpeedNextTime += gameSpeedWaitTime;

				// ���C������
				if( GameMain(msg.hwnd) == false ){
					break; // �I��������
				}
			}else{
				// �\�����FPS�`�悵�Ȃ���΂Ȃ�Ȃ����`�F�b�N����
				if(currentTime >= viewSpeedNextTime ){
					viewSpeedNextTime += viewSpeedWaitTime;

					onPaint(g_hWnd, g_mDC);
					::InvalidateRect(g_hWnd, NULL, FALSE);
					g_frame++;
				}else{
					Sleep(1);
				}
			}
		}
	}

	// ���낢���Еt��������
	trace(L"�I������\n");
	GameMain_quit();

	::timeEndPeriod(1);
	return (int) msg.wParam;
}



//
//  �֐�: MyRegisterClass()
//
//  �ړI: �E�B���h�E �N���X��o�^���܂��B
//
//  �R�����g:
//
//    ���̊֐�����юg�����́A'RegisterClassEx' �֐����ǉ����ꂽ
//    Windows 95 ���O�� Win32 �V�X�e���ƌ݊�������ꍇ�ɂ̂ݕK�v�ł��B
//    �A�v���P�[�V�������A�֘A�t����ꂽ
//    �������`���̏������A�C�R�����擾�ł���悤�ɂ���ɂ́A
//    ���̊֐����Ăяo���Ă��������B
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETRIS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TETRIS);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �֐�: InitInstance(HINSTANCE, int)
//
//   �ړI: �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g:
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   g_hWnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   return TRUE;
}


//
//  �֐�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �ړI:  ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND	- �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT	- ���C�� �E�B���h�E�̕`��
//  WM_DESTROY	- ���~���b�Z�[�W��\�����Ė߂�
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HWND desktop;
	static HBITMAP hBitmap = NULL;
	RECT rect;

	switch (message)
	{
	case WM_CREATE:
		desktop = ::GetDesktopWindow();
		::GetWindowRect(desktop, &rect);
		
		::SetWindowPos(hWnd, NULL, rect.right / 4, (rect.bottom - rect.top) / 4, 510, 510, 0);

		::GetClientRect(hWnd, &g_windowRect);
		::FillRect(::GetDC(hWnd), &g_windowRect, ::CreateSolidBrush(RGB(0,0,0)));

		// ������DC�̍쐬
		{
			HDC hdc = ::GetDC(hWnd);

			g_mDC = ::CreateCompatibleDC(hdc); // ������DC�̍쐬
			hBitmap = ::CreateCompatibleBitmap(hdc, g_windowRect.right, g_windowRect.bottom); // ������DC�p�̃r�b�g�}�b�v�쐬
			::SelectObject(g_mDC, hBitmap); // �֘A�t��
		}
		break;
	case WM_KEYDOWN:
		onKeyDown(wParam);
		::InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, g_windowRect.left, g_windowRect.top, g_windowRect.right, g_windowRect.bottom, g_mDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		::GetClientRect(hWnd, &g_windowRect);
		
		if(wParam == SIZE_RESTORED){ // �E�C���h�E�̃T�C�Y�ύX��
			::GetClientRect(hWnd, &g_windowRect);

			// �����̃�����DC������΂����j��
			if( g_mDC != NULL ){
				::DeleteObject(::g_mDC); // mDC��j��
				::DeleteObject(hBitmap); // �֘A����bitmap���j��
			}

			// ������DC�̍쐬
			{
				HDC hdc = ::GetDC(hWnd);
				g_mDC = ::CreateCompatibleDC(hdc); // ������DC�̍쐬
				hBitmap = ::CreateCompatibleBitmap(hdc, g_windowRect.right, g_windowRect.bottom); // ������DC�p�̃r�b�g�}�b�v�쐬
				::SelectObject(g_mDC, hBitmap); // �֘A�t��
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		return FALSE;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
