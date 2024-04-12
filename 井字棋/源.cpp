#include<graphics.h>
char board_data[3][3] = {
	{'-','-','-'},
	{'-','-','-'},
	{'-','-','-'},
};
char current_piece = 'O';
bool checkwin(char c)
{
	if (board_data[0][0] == c && board_data[0][1] == c && board_data[0][2] == c)
	{
		return true;
	}
	if (board_data[1][0] == c && board_data[1][1] == c && board_data[1][2] == c)
	{
		return true;
	}
	if (board_data[2][0] == c && board_data[2][1] == c && board_data[2][2] == c)
	{
		return true;
	}
	if (board_data[0][0] == c && board_data[1][0] == c && board_data[2][0] == c)
	{
		return true;
	}
	if (board_data[0][1] == c && board_data[1][1] == c && board_data[2][1] == c)
	{
		return true;
	}
	if (board_data[0][2] == c && board_data[1][2] == c && board_data[2][2] == c)
	{
		return true;
	}
	if (board_data[0][0] == c && board_data[1][1] == c && board_data[2][2] == c)
	{
		return true;
	}
	if (board_data[2][0] == c && board_data[1][1] == c && board_data[0][2] == c)
	{
		return true;
	}
	return false;
}
bool checkdraw()
{
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			if (board_data[i][j] == '-')
			{
				return false;
			}
		}
	}
	return true;
}
void drawboard()
{
	line(0, 200, 600, 200);
	line(0, 400, 600, 400);
	line(200, 0, 200, 600);
	line(400, 0, 400, 600);
}
void drawpiece()
{
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			switch (board_data[i][j])
			{
			case'O':
				circle(200 * j + 100, 200 * i + 100, 100);
				break;
			case'X':
				line(200 * j, 200 * i, 200 * (j + 1), 200 * (i + 1));
				line(200 * (j + 1), 200 * i, 200 * j, 200 * (i + 1));
				break;
			case'-':
				break;
			}
		}
	}
}
void drawtiptext()
{
	static TCHAR str[64];
	_stprintf_s(str, _T("当前棋子类型：%c"),current_piece);
	settextcolor(RGB(225, 175, 45));
	outtextxy(0, 0, str);
}
int main()
{
	initgraph(600, 600);


	bool running = true;
	ExMessage msg;
	BeginBatchDraw();
	while (running)
	{
		DWORD start_time = GetTickCount();
		while (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN)
			{
				int x = msg.x;
				int y = msg.y;
				int index_x = x / 200;
				int index_y = y / 200;
				if (board_data[index_y][index_x] == '-')
				{
					board_data[index_y][index_x] = current_piece;
					if (current_piece == 'O')
					{
						current_piece = 'X';
					}
					else
					{
						current_piece = 'O';
					}
				}
			}
		}
		cleardevice();
		drawboard();
		drawpiece();
		drawtiptext();
		FlushBatchDraw();
		if (checkwin('X'))
		{
			MessageBox(GetHWnd(), _T("X player wins"), _T("game over"), MB_OK);
			running = false;
		}
		else if (checkwin('O'))
		{
			MessageBox(GetHWnd(), _T("O player wins"), _T("game over"), MB_OK);
			running = false;
		}
		else if (checkdraw())
		{
			MessageBox(GetHWnd(), _T("draw"), _T("game over"), MB_OK);
			running = false;
		}
		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 60)
		{
			Sleep(1000 / 60 - delta_time);
		}
	}
	EndBatchDraw();
	return 0;
}