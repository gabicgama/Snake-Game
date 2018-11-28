#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_SAMPLE *sound_effect1 = NULL, *sound_effect2 = NULL, *sound_effect3 = NULL, *soundtrack = NULL;

	int width = 800;
	int height = 600;
	const float FPS = 60.0;
	const float frameFPS = 15.0;

	if (!al_init()) return -1; //verifica se allegro iniciou
	display = al_create_display(width, height); //cria janela
	if (!display) return -1; //verifica se tela iniciou
	al_set_window_position(display, 250, 50); //posiciona a janela
	al_set_window_title(display, "Snake Game"); //add titulo a janela

	int x = 0, y = 0;

	//iniciando as bibliotecas
	al_init_primitives_addon();
	al_install_keyboard();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_install_audio();
	al_init_acodec_addon();

	sound_effect1 = al_load_sample("sound/game-coin.wav");
	sound_effect2 = al_load_sample("sound/game-over03.wav");
	sound_effect3 = al_load_sample("sound/game-start-sound-effect-gun-reload-and-jump.wav");
	soundtrack = al_load_sample("sound/happy-8bit-loop.ogg");

	if (!al_reserve_samples(4))
	{
		fprintf(stderr, "failed to reserve samples!\n");
		return -1;
	}

	//colocando música de fundo
	ALLEGRO_SAMPLE_INSTANCE *songInstance = al_create_sample_instance(soundtrack);
	al_set_sample_instance_playmode(songInstance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(songInstance, al_get_default_mixer());
	al_play_sample_instance(songInstance);

	//carregando imagens
	ALLEGRO_BITMAP *bg = al_load_bitmap("img/background.png");
	ALLEGRO_BITMAP *apple = al_load_bitmap("img/apple.jpg");
	ALLEGRO_BITMAP *body = al_load_bitmap("img/body.jpg");
	ALLEGRO_BITMAP *head = al_load_bitmap("img/head.jpg");
	ALLEGRO_BITMAP *bgmenu = al_load_bitmap("img/bgmenu.jpg");
	ALLEGRO_FONT *font1 = al_load_font("ttf/PressStart2P.ttf", 10, 0);
	ALLEGRO_FONT *font2 = al_load_font("ttf/PressStart2P.ttf", 20, 0);

	//temporizadores e fps
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 10); //velocidade
	ALLEGRO_TIMER *frameTimer = al_create_timer(1.0 / frameFPS);
	ALLEGRO_TIMER *VREME = al_create_timer(1);
	ALLEGRO_KEYBOARD_STATE keyState;
	ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();

	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_timer_event_source(frameTimer));
	al_register_event_source(event_queue, al_get_timer_event_source(VREME));

	al_start_timer(timer);
	al_start_timer(frameTimer);
	al_start_timer(VREME);
	srand(time(NULL));

	int timeS = 0, timeF = 0, time = 0;

	enum Direction { DOWN, LEFT, RIGHT, UP };//define conjunto de valores para dir

	int dir = DOWN;
	int score = 1;
	int lastX;
	int lastY;

	int appleX = 40 * (rand() % 20);
	int appleY = 40 * (rand() % 15);

	int snakeX[50], snakeY[50];

	int done = 0;
	int menu = 1;
	int dead = 0;
	int draw = 1;

	while (!done)
	{
		lastX = x;
		lastY = y;

		ALLEGRO_EVENT events;
		al_wait_for_event(event_queue, &events);

		if (events.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch (events.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				done = 1;
				break;
			case ALLEGRO_KEY_ENTER:
				if (menu) menu = 0, score = 1, timeS = 0, x = 0, y = 0;
				al_play_sample(sound_effect3, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); //som de start
				break;
			}
		}
		else if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE)//clicar em fechar
			done = 1;

		if (events.type == ALLEGRO_EVENT_TIMER)
		{
			if (events.timer.source == VREME) timeS++;
			if (events.timer.source == timer)
			{

				al_get_keyboard_state(&keyState);
				if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT) && dir != LEFT)
					dir = RIGHT;
				else if (al_key_down(&keyState, ALLEGRO_KEY_LEFT) && dir != RIGHT)
					dir = LEFT;
				else if (al_key_down(&keyState, ALLEGRO_KEY_UP) && dir != DOWN)
					dir = UP;
				else if (al_key_down(&keyState, ALLEGRO_KEY_DOWN) && dir != UP)
					dir = DOWN;
				else if (al_key_down(&keyState, ALLEGRO_KEY_ENTER) && menu == 1/*true*/)
					menu = 0, score = 1, timeS = 0, x = 0, y = 0;

				if (menu == 0/*false*/)
				{
					if (score != 0)
					{
						for (int i = score; i > 0; i--)//aumenta posiçoes do vetor de corpo de acordo com a pontuacao
						{
							snakeX[i] = snakeX[i - 1];
							snakeY[i] = snakeY[i - 1];
						}
						snakeX[0] = lastX;
						snakeY[0] = lastY;
					}
				}

				switch (dir)
				{
				case RIGHT:
					x = x + 40;
					break;
				case LEFT:
					x = x - 40;
					break;
				case UP:
					y = y - 40;
					break;
				case DOWN:
					y = y + 40;
					break;
				}

				if (x == appleX && y == appleY)//colisao cobra com maça
				{
					score++;
					al_play_sample(sound_effect1, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, NULL);
					appleX = 40 * (rand() % 20);
					appleY = 40 * (rand() % 15);
				}

				if (menu == 0/*false*/)//verifica se colisao com fim da tela e com o corpo
				{
					for (int i = 0; i < score; i++)
					{
						if (x == snakeX[i] && y == snakeY[i] && menu == 0/*false*/) dead = 1/*true*/;
					}
					if (x < 0 || x >= 800 || y < 0 || y >= 600 && menu == 0/*false*/) dead = 1/*true*/;
				}
				draw = 1/*true*/;
			}
		}

		if (dead && menu == 0/*false*/)
		{
			menu = true;
			timeF = timeS;
			x = 0, y = 0;
			dead = 0/*false*/;
			dir = DOWN;

			al_play_sample(sound_effect2, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, NULL); //game over
		}

		//menu do game
		if (draw == 1/*true*/)
		{
			draw = 0/*false*/;
			if (menu)
			{
				x = 0, y = 0;
				if (time > 10) time = 0;
				al_draw_bitmap(bgmenu, 0, 0, 0);
				if (time < 5) {
					al_draw_text(font2, al_map_rgb(0, 0, 0), width / 2, 230, ALLEGRO_ALIGN_CENTRE,
						"Press Enter to Start");
					al_draw_text(font2, al_map_rgb(0, 0, 0), width / 2, 280, ALLEGRO_ALIGN_CENTRE,
						"Press Esc to Exit");
				}
				time++;
				al_draw_textf(font2, al_map_rgb(0, 0, 0), width/2, 430, ALLEGRO_ALIGN_CENTRE,
					"Points: %i", score - 1);
				al_draw_textf(font2, al_map_rgb(0, 0, 0), width/2, 480, ALLEGRO_ALIGN_CENTRE,
					"Time: %i sec", timeF);

					int highscore;
					FILE * arq = fopen("highscore.txt","r");
					fscanf( arq, "%d", &highscore );
					fclose(arq);

					if(score > highscore){
                        FILE * arq = fopen("highscore.txt","w");
                        fprintf (arq, "%d", score-1);
                        fclose(arq);
					}

                al_draw_textf(font2, al_map_rgb(233, 29, 41), width/2, 350, ALLEGRO_ALIGN_CENTRE,
					"Highscore: %i", highscore);

			}
			else
			{
				al_draw_bitmap(apple, appleX, appleY, NULL);//maça

				for (int i = 0; i < score; i++)
				{
					al_draw_bitmap(body, snakeX[i], snakeY[i], NULL);//snake
				}

				al_draw_bitmap(head, x, y, NULL);

				al_draw_textf(font1, al_map_rgb(0, 0, 0), 20, 5, 0,
					"Points: %i", score - 1);
				al_draw_textf(font1, al_map_rgb(0, 0, 0), 670, 5, 0,
					"Time: %i", timeS);

			}

			al_flip_display();
			al_draw_bitmap(bg, 0, 0, 0);
		}
	}//fim while

	al_destroy_sample(soundtrack);
	al_destroy_sample_instance(songInstance);
	al_destroy_sample(sound_effect3);
	al_destroy_sample(sound_effect2);
	al_destroy_sample(sound_effect1);
	al_destroy_bitmap(apple);
	al_destroy_bitmap(head);
	al_destroy_bitmap(body);
	al_destroy_bitmap(bg);
	al_destroy_bitmap(bgmenu);
	al_destroy_event_queue(event_queue);
	al_destroy_timer(timer);
	al_destroy_display(display);

	return 0;
}
