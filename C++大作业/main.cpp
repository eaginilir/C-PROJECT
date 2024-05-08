#include<easyx.h>
#include<string>
#include<vector>
#include<windows.h>
#include<cmath>

const int window_width = 1280;
const int window_height = 720;

const int button_width = 192;
const int button_height = 75;

const double pi = 3.14159;



#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

bool is_game_started = false;
bool running = true;

//渲染画面
inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

//定义Atlas类
class Atlas
{
public:
	
	Atlas(LPCTSTR path,int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)		
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	};

	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	};

public:
	std::vector<IMAGE*>frame_list;
};

Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;


//定义Button类
class Button
{
public:
	Button(RECT rect,LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect;
		
		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	};

	~Button()=default;

	void ProcessEvent(const ExMessage&msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Hovered;
			}
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Idle;
			}
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Pushed;
			}
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
			{
				OnClick();
				break;
			}
		default:
			break;
		}
	};

	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	};

protected:
	//所有继承Button类的类都必须实现自己的OnClick函数
	virtual void OnClick() = 0;

private:
	//枚举Button的状态
	enum class Status
	{
		Idle=0,
		Hovered,
		Pushed
	};


private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status=Status::Idle;

private:
	//检测鼠标点击
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	};

};

//开始游戏按钮
class StartGameButton :public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}

	~StartGameButton() = default;

protected:
	void OnClick()
	{
		is_game_started = true;
	};
};

//退出游戏按钮
class QuitGameButton :public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}

	~QuitGameButton() = default;

protected:
	void OnClick()
	{
		running = false;
	};
};


//定义Animation类
class Animation
{
public:
	Animation(Atlas* atlas, int interval)
	{
		interval_ms = interval;
		anim_atlas = atlas;
	};

	~Animation() = default;

	void play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}
		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	};
private:
	int interval_ms = 0;
	int timer = 0;//计时器 
	int idx_frame = 0;//动画帧数

private:
	Atlas* anim_atlas;

};

//定义Bullet类
class Bullet
{
public:
	POINT position = { 0,0 };

public:
	Bullet() = default;
	~Bullet() = default;

	void draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	};

private:
	const int RADIUS = 10;
};

//定义圆周子弹
class circle_bullet :public Bullet
{
public:
	void draw()const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	};

private:
	const int RADIUS = 10;
};



//定义Player类
class Player
{
public:
	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(atlas_player_left,45);
		anim_right = new Animation(atlas_player_right,45);
		init_bullet(bullet_list,circle_bullet_list);
	};

	~Player()
	{
		delete anim_left;
		delete anim_right;
	};

	void processevent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_KEYDOWN:
			switch (msg.vkcode)
			{
			case 'W':
				is_move_up = true;
				break;
			case 'A':
				is_move_left = true;
				break;
			case 'S':
				is_move_down = true;
				break;
			case 'D':
				is_move_right = true;
				break;
			case VK_SPACE:
				skill = true;
				break;
			default:
				break;
			}
			break;
		case WM_KEYUP:
			switch (msg.vkcode)
			{
			case 'W':
				is_move_up = false;
				break;
			case 'A':
				is_move_left = false;
				break;
			case 'S':
				is_move_down = false;
				break;
			case 'D':
				is_move_right = false;
				break;
			case VK_SPACE:
				skill = false;
				break;
			default:
				break;
			}
			break;
		}
	};

	void move()
	{
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_position.x += (int)(speed * normalized_x);
			player_position.y += (int)(speed * normalized_y);
		}
		if (player_position.x < 0)
		{
			player_position.x = 0;
		}
		if (player_position.y < 0)
		{
			player_position.y = 0;
		}
		if (player_position.x + frame_width > window_width)
		{
			player_position.x = window_width - frame_width;
		}
		if (player_position.y + frame_height > window_height)
		{
			player_position.y = window_height - frame_height;
		}
	};

	void draw(int delta)
	{
		int pos_shadow_x = player_position.x + (frame_width / 2 - shadow_width / 2);
		int pos_shadow_y = player_position.y + frame_height - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
		{
			facing_left = true;
		}
		else if (dir_x > 0)
		{
			facing_left = false;
		}

		if (facing_left)
		{
			anim_left->play(player_position.x, player_position.y, delta);
		}
		else
		{
			anim_right->play(player_position.x, player_position.y, delta);
		}
	};

	const POINT& get_position() const
	{
		return player_position;
	}

	void init_bullet(std::vector<Bullet>& bullet_list,std::vector<circle_bullet>&circle_bullet_list)
	{
		for (int i = 0; i < 2; i++)
		{
			Bullet new_bullet;
			circle_bullet new_circle_bullet;
			bullet_list.push_back(new_bullet);
			circle_bullet_list.push_back(new_circle_bullet);
		}
	}

	void add_bullet(std::vector<Bullet>&bullet_list,std::vector<circle_bullet>&circle_bullet_list)
	{
		int i = rand() % 2;
		if (i == 0)
		{
			Bullet new_bullet;
			bullet_list.push_back(new_bullet);
		}
		else
		{
			circle_bullet new_bullet;
			circle_bullet_list.push_back(new_bullet);
		}
	}

	bool release_skill()
	{
		return skill && sp == 100;
	}
	

public:
	const int speed = 3;
	const int frame_width = 55;
	const int frame_height = 92;
	const int shadow_width = 32;
	const int window_width = 1280;
	const int window_height = 720;
	std::vector<Bullet> bullet_list;
	std::vector<circle_bullet> circle_bullet_list;
	int grade = 1;//定义等级
	int hp = 100;//定义血量
	int sp = 0;//定义能量

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT player_position = { 500,500 };
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
	bool skill = false;
	friend Bullet;
	friend circle_bullet;

};

//定义Enemy类
class Enemy
{
public:
	Enemy() 
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);
		
		//敌人生成边界
		enum class SpawnEdge
		{
			up = 0,
			down,
			left,
			right
		};

		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		
		switch (edge)
		{
		case SpawnEdge::up:
			position.x = rand() % window_width;
			position.y = -frame_height;
			break;
		case SpawnEdge::down:
			position.x = rand() % window_width;
			position.y = window_height;
			break;
		case SpawnEdge::left:
			position.x = -frame_width;
			position.y = rand() % window_height;
			break;
		case SpawnEdge::right:
			position.x = window_width;
			position.y = rand() % window_height;
			break;
		default:
			break;
		}

		birth_time = GetTickCount();
	};

	bool check_bullet_collision(const Bullet& bullet)
	{
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + frame_width;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + frame_height;
		return is_overlap_x && is_overlap_y;
	}

	bool check_player_collision(const Player& player)
	{
		POINT check_position = { position.x + frame_width / 2,position.y + frame_height / 2 };
		POINT player_position = player.get_position();
		bool is_overlap_x = check_position.x >= player_position.x && check_position.x <= player_position.x + player.frame_width;
		bool is_overlap_y = check_position.y >= player_position.y && check_position.y <= player_position.y + player.frame_height;
		return is_overlap_x && is_overlap_y;
	}

	void move(const Player& player)
	{
		const POINT& player_position = player.get_position();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(speed * normalized_x);
			position.y += (int)(speed * normalized_y);
		}

		if (dir_x < 0)
		{
			facing_left = true;
		}
		else if (dir_x > 0)
		{
			facing_left = false;
		}
	}

	void draw(int delta)
	{
		int pos_shadow_x = position.x + (frame_width / 2 - shadow_width / 2);
		int pos_shadow_y = position.y + frame_height - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
		{
			anim_left->play(position.x, position.y, delta);
		}
		else
		{
			anim_right->play(position.x, position.y, delta);
		}
	}

	~Enemy() 
	{
		delete anim_left;
		delete anim_right;
	};

	void hurt()
	{
		alive = false;
	};

	bool checkalive()
	{
		return alive;
	};

	bool check_time()
	{
		DWORD time = GetTickCount();
		DWORD delta_time = time - birth_time;
		if (delta_time != 0 && delta_time % 3000 == 0)
		{
			return true;
		}
		return false;
	}

	const POINT& get_position() const
	{
		return position;
	}



private:
	const int speed = 2;
	const int frame_width = 80;
	const int frame_height = 80;
	const int shadow_width = 48;


public:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
	bool alive = true;
	DWORD birth_time;


};

//定义敌人子弹类
class enemy_bullet :public Bullet
{
public:

	void init_nor_x_and_nor_y(POINT& enemy_position,POINT &player_position)
	{
		int dir_x = player_position.x - enemy_position.x;
		int dir_y = player_position.y - enemy_position.y;
		double dir_len = sqrt(pow(dir_x, 2) + pow(dir_y, 2));
		normalized_x = dir_x / dir_len;
		normalized_y = dir_y / dir_len;
	};

	void set_position(POINT& enemy_position)
	{
		position.x = enemy_position.x + 40;
		position.y = enemy_position.y + 40;
	};

	void draw()const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(255, 0, 0));
		fillcircle(position.x, position.y, RADIUS);
	};

	//检测子弹撞墙
	bool check_collision()
	{
		bool is_collision_x = position.x<0 || position.x>window_width;
		bool is_collision_y = position.y<0 || position.y>window_height;
		return is_collision_x || is_collision_y;
	};

	//检测与玩家相撞
	bool check_player_collision(Player& player)
	{
		POINT player_position = player.get_position();
		bool is_collision_x = position.x > player_position.x + 5 && position.x < player_position.x + 35;
		bool is_colloision_y= position.y > player_position.y && position.y < player_position.y + 80;
		return is_collision_x && is_colloision_y;
	}

public:
	double normalized_x;
	double normalized_y;

private:
	const int RADIUS = 10;



};

//生成新敌人
void generate_enemy(std::vector<Enemy*>& enemy_list,Player &player)
{
	const int interval = max(100 - 3 * player.grade, 10);
	static int counter = 0;
	if ((++counter) % interval == 0)
	{
		enemy_list.push_back(new Enemy());
	}
}

//更新螺旋子弹位置
void update_bullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	const double radial_speed = 0.0045;//径向速度
	const double tangent_speed = 0.0055;//切向速度
	double radian_interval = 2 * pi / bullet_list.size();//子弹之间的弧度间隔
	POINT player_position = player.get_position();
	double radius = 100 + 25 * sin(GetTickCount() * radial_speed);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * tangent_speed + radian_interval * i;//当前子弹所在弧度
		bullet_list[i].position.x = player_position.x + player.frame_width / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.frame_height / 2 + (int)(radius * cos(radian));
	}
}

//更新圆周子弹位置
void update_bullets(std::vector<circle_bullet>& circle_bullet_list, const Player& player)
{
	const double tangent_speed = 0.002;//切向速度
	double radian_interval = 2 * pi / circle_bullet_list.size();//子弹间隔弧度
	POINT player_position = player.get_position();
	double radius = 100;
	for (size_t i = 0; i < circle_bullet_list.size(); i++)
	{
		double radian = GetTickCount() * tangent_speed + radian_interval * i;//子弹所在弧度
		circle_bullet_list[i].position.x = player_position.x + player.frame_width / 2 + (int)(radius * sin(radian));
		circle_bullet_list[i].position.y = player_position.y + player.frame_height / 2 + (int)(radius * cos(radian));
	}
}

//更新敌人子弹位置
void update_bullets(std::vector<enemy_bullet*>& enemy_bullet_list)
{
	const double speed = 5;
	for (size_t i = 0; i < enemy_bullet_list.size(); i++)
	{
		enemy_bullet_list[i]->position.x += enemy_bullet_list[i]->normalized_x * speed;
		enemy_bullet_list[i]->position.y += enemy_bullet_list[i]->normalized_y * speed;
	}
}

//绘制玩家得分
void draw_player_score(int score)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分:%d"), score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}

//绘制玩家等级
void draw_player_grade(int grade)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家等级:%d"), grade);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 30, text);
}

//绘制玩家血量
void draw_player_hp(Player& player)
{
	setlinecolor(RGB(255, 255, 255));
	setfillcolor(RGB(255, 0, 0));
	rectangle(10, 50, 210, 60);
	fillrectangle(10, 50, 10 + 2*player.hp, 60);
}

//绘制玩家能量条
void draw_player_sp(Player& player)
{
	setlinecolor(RGB(255, 255, 255));
	setfillcolor(RGB(0, 0, 255));
	rectangle(10, 80, 210, 90);
	fillrectangle(10, 80, min(10 + 2 * player.sp,210), 90);
}

//绘制玩家释放技能
void draw_skill(Player& player,std::vector<enemy_bullet*>&enemy_bullet_list)
{
	player.hp = min(100, player.hp + 10);
	for (auto& bullet : enemy_bullet_list)
	{
		delete bullet;
	}
	enemy_bullet_list.clear();
}

int main()
{
	//创建窗口
	initgraph(1080, 720);


	//加载音乐并取名
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);

	mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);

	atlas_player_left = new Atlas(_T("img/player_left_%d.png"), 6);
	atlas_player_right = new Atlas(_T("img/player_right_%d.png"), 6);
	atlas_enemy_left = new Atlas(_T("img/enemy_left_%d.png"), 3);
	atlas_enemy_right = new Atlas(_T("img/enemy_right_%d.png"), 3);

	//定义变量
	int score = 0;
	ExMessage msg;
	IMAGE background;
	Player player;
	IMAGE img_menu;
	std::vector<Enemy*> enemy_list;
	std::vector<enemy_bullet*>enemy_bullet_list;




	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (window_width - button_width) / 2 + 240;
	region_btn_start_game.right = region_btn_start_game.left + button_width;
	region_btn_start_game.top = 330;
	region_btn_start_game.bottom = region_btn_start_game.top + button_height;

	region_btn_quit_game.left = (window_width - button_width) / 2 + 240;
	region_btn_quit_game.right = region_btn_quit_game.left + button_width;
	region_btn_quit_game.top = 450;
	region_btn_quit_game.bottom = region_btn_quit_game.top + button_height;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
		_T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));


	loadimage(&img_menu, _T("img/menu.png"));
	loadimage(&background, _T("img/background.png"));
	BeginBatchDraw();

	while (running)
	{
		DWORD begin_time = GetTickCount();

		while (peekmessage(&msg))
		{
			if (is_game_started)
			{
				player.processevent(msg);
			}
			else
			{
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}
		if (is_game_started)
		{
			player.move();

			//更新子弹位置 
			update_bullets(player.bullet_list, player);
			update_bullets(player.circle_bullet_list, player);
			update_bullets(enemy_bullet_list);

			generate_enemy(enemy_list,player);
			for (Enemy* enemy : enemy_list)
			{
				enemy->move(player);
				//敌人生成子弹
				if (enemy->check_time())
				{
					enemy_bullet* new_bullet = new enemy_bullet();
					POINT enemy_position = enemy->get_position();
					POINT player_position = player.get_position();
					new_bullet->set_position(enemy_position);
					new_bullet->init_nor_x_and_nor_y(enemy_position, player_position);
					enemy_bullet_list.push_back(new_bullet);
				}
			}

			//检测敌人和玩家碰撞
			for (size_t i=0;i<enemy_list.size();i++)
			{
				if (enemy_list[i]->check_player_collision(player))
				{
					Enemy* this_enemy = enemy_list[i];
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete this_enemy;
					player.hp = player.hp - 20;
					if (player.hp <= 0)
					{
						static TCHAR text[128];
						_stprintf_s(text, _T("最终得分:%d!"), score);
						MessageBox(GetHWnd(), text, _T("对不起，你没能打赢复活赛！"), MB_OK);
						running = false;
						break;
					}
				}
			}

			//检测玩家和敌人子弹碰撞
			for (size_t i = 0; i < enemy_bullet_list.size(); i++)
			{
				if (enemy_bullet_list[i]->check_player_collision(player))
				{
					enemy_bullet* bullet = enemy_bullet_list[i];
					std::swap(enemy_bullet_list[i], enemy_bullet_list.back());
					enemy_bullet_list.pop_back();
					delete bullet;
					player.hp = player.hp - 10;
					if (player.hp<=0) 
					{
						static TCHAR text[128];
						_stprintf_s(text, _T("最终得分:%d!"), score);
						MessageBox(GetHWnd(), text, _T("对不起，你没能打赢复活赛！"), MB_OK);
						running = false;
						break;
					}
				}
			}

			//检测敌人和子弹碰撞
			for (Enemy* enemy : enemy_list)
			{
				for (const Bullet& bullet : player.bullet_list)
				{
					if (enemy->check_bullet_collision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						enemy->hurt();
						score++;
						player.sp = min(player.sp + 10, 100);
						//更新玩家等级
						if (score % 10 == 0)
						{
							player.grade++;
							player.add_bullet(player.bullet_list, player.circle_bullet_list);
						}
					}
				}
				for (const Bullet& bullet : player.circle_bullet_list)
				{
					if (enemy->check_bullet_collision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						enemy->hurt();
						score++;
						player.sp = min(player.sp + 10, 100);
						//更新玩家等级
						if (score % 10 == 0)
						{
							player.grade++;
							player.add_bullet(player.bullet_list, player.circle_bullet_list);
						}
					}
				}
			}

			//清除碰到墙壁的子弹
			for (size_t i = 0; i < enemy_bullet_list.size(); i++)
			{
				enemy_bullet* bullet = enemy_bullet_list[i];
				if (bullet->check_collision())
				{
					std::swap(enemy_bullet_list[i], enemy_bullet_list.back());
					enemy_bullet_list.pop_back();
					delete bullet;
				}
			}

			//清除生命值为0的敌人
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->checkalive())
				{
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}

			//检测玩家是否释放技能
			if (player.release_skill())
			{
				draw_skill(player,enemy_bullet_list);
				player.sp = 0;
			}
		}

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time-begin_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}

		cleardevice();
		if (is_game_started)
		{
			putimage_alpha(0, 0, &background);
			player.draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
			{
				enemy->draw(1000 / 144);
			}
			for (const Bullet& bullet : player.bullet_list)
			{
				bullet.draw();
			}
			for (const circle_bullet& circle_bullet : player.circle_bullet_list)
			{
				circle_bullet.draw();
			}
			for (enemy_bullet*& enemy_bullet : enemy_bullet_list)
			{
				enemy_bullet->draw();
			}

			//绘制玩家分数、等级和血量
			draw_player_score(score);
			draw_player_grade(player.grade);
			draw_player_hp(player);
			draw_player_sp(player);
		}
		else
		{
			putimage(0, 0,&img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}
		FlushBatchDraw();

	}

	delete atlas_player_left;
	delete atlas_player_right;


	EndBatchDraw();

	return 0;
}	