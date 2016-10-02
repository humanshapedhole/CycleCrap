// ПОДКЛЮЧЕНИЕ 1-ГО СДВИГОВОГО РЕГИСТРА 74НС595
#define DATA					3  		// DS подключен к 3 пину
#define LATCH       			4  		// ST_CP подключен к 4 пину
#define CLOCK	      			5  		// SH_CP подключен к 5 пину
// ПОДКЛЮЧЕНИЕ 2-ГО СДВИГОВОГО РЕГИСТРА 74НС595
//#define DATA					3+3		// DS подключен к 6 пину
//#define LATCH       			4+3		// ST_CP подключен к 7 пину
//#define CLOCK       			5+3		// SH_CP подключен к 8 пину

// ПОДКЛЮЧЕНИЕ ДАТЧИКА ХОЛЛА
#define SENSOR       			0	   	// датчик Холла подключен ко 2 пину
										// 0 -  прерывание

// ПЛОТНОСТЬ ПИКСЕЛЕЙ
#define STEPS_NUMBER			180		// установка числа разбиений окружности (эмпирич.)
#define SHIFT					0		// сдвиг надписи						(эмпирич.)
#define LINE_WIDTH				24
#define LINE_HEIGHT				8

// ШАБЛОН ВЫВОДИМОГО СИМВОЛА
byte A[LINE_HEIGHT][LINE_WIDTH] = 
{
		{0, 0, 0, 0, 0, 0},				// верхняя строка будет снизу
		{1, 0, 0, 0, 0, 0},
		{1, 1, 0, 0, 0, 1},
		{1, 1, 1, 0, 1, 1},
		{1, 1, 1, 1, 1, 1},
		{1, 1, 1, 0, 1, 1},
		{1, 1, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0}
};

byte X[LINE_HEIGHT][LINE_WIDTH] = 
{
		{0, 0, 0, 0, 0, 0},				// верхняя строка будет снизу
		{1, 0, 0, 0, 0, 0},
		{0, 1, 0, 0, 0, 1},
		{0, 0, 1, 0, 1, 0},
		{0, 0, 0, 1, 0, 0},
		{0, 0, 1, 0, 1, 0},
		{0, 1, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0}
};

byte POPA[LINE_HEIGHT][LINE_WIDTH] = 
{
		{1, 1, 1, 1, 1, 0,  0, 1, 1, 1, 0, 0,  1, 1, 1, 1, 1, 0,  0, 0, 1, 0, 0, 0},	
		{1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  0, 1, 0, 1, 0, 0},
		{1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  0, 1, 0, 1, 0, 0},
		{1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0},
		{1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 1, 1, 1, 1, 0},
		{1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0},
		{1, 0, 0, 0, 1, 0,  0, 1, 1, 1, 0, 0,  1, 0, 0, 0, 1, 0,  1, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0}
};

/*
const int symbols_number 	= 1;		// количество символов для отображения
const int symbols_width 	= 6;		// ширина символа в пикселях
*/
// ******************************************************************************************  
// ПОЛЬЗОВАТЕЛЬСКИЕ ПЕРЕМЕННЫЕ:

unsigned long lap_time;           		// интервал между срабатываниями датчика Холла 	(мкс)
unsigned long current_time;    			// время в микросекундах на начало оборота 		(мкс)

unsigned long T;                  		// задержка (таумаут) 							(мкс)
int k;                            		// количество таймаутов
int	starting_point[2];             		// начальная точка отображения символа (для 0 и 1 LL)
int	final_point[2];            			// конечная точка отображения символа  (для 0 и 1 LL)

bool rotating;							// идентификатор вращения колеса
bool interrupted;						// 

// ****************************************************************************************** 
// ПОЛЬЗОВАТЕЛЬСКИЕ ФУНКЦИИ:

// ******************************************************************************************
// ПРЕДУСТАНОВКИ: 
void setup() 
{
	// определение входа датчика
	pinMode(SENSOR, INPUT);
	Serial.begin(9600); 
	
	//устанавливаем режим OUTPUT
	pinMode(LATCH, OUTPUT);
	pinMode(CLOCK, OUTPUT);
	pinMode(DATA, OUTPUT);
	
	pinMode(LATCH+3, OUTPUT);
	pinMode(CLOCK+3, OUTPUT);
	pinMode(DATA+3, OUTPUT);
	// тушим диоды
	digitalWrite(LATCH, LOW);
	shiftOut(DATA, CLOCK, MSBFIRST, 0); 
	digitalWrite(LATCH, HIGH);
	
	digitalWrite(LATCH+3, LOW);
	shiftOut(DATA+3, CLOCK+3, MSBFIRST, 0); 
	digitalWrite(LATCH+3, HIGH);
	
	// начальные значения датчика оборотов
	rotating = false;               	// колесо не крутится
	lap_time = 0;                   	// первоначальное время оборота равно нулю
	current_time = 0;					// текущее время на начало первого оборота равно нулю
	T = 0;                       		// начальное значение задержки 
	interrupted = false;

	// настройка прерывания
	attachInterrupt(SENSOR, reset, RISING);
	
}

// ******************************************************************************************
// ПРЕРЫВАНИЕ: 
void reset()
{
	lap_time = micros() - current_time;
	current_time = micros();
	rotating = true;
	interrupted = true;
	T = lap_time / STEPS_NUMBER;
	//Serial.print("delay time: "); Serial.println(T);
	
	for (int i=0; i<2; i++)
	{
		starting_point[i] = i*STEPS_NUMBER/2 + STEPS_NUMBER/4 - LINE_WIDTH/2 + T*SHIFT;
		final_point[i] = starting_point[i] + LINE_WIDTH + T*SHIFT;
	}
	/* РАЗВЕРНУТО:
	starting_point[0] = STEPS_NUMBER/4 - LINE_WIDTH/2;
	starting_point[1] = 3*STEPS_NUMBER/4 - LINE_WIDTH/2;
	final_point[0] = starting_point[0] + LINE_WIDTH;	
	final_point[1] = starting_point[1] + LINE_WIDTH;
	*/
}

// ******************************************************************************************
// ЦИКЛ: 
void loop() 
{
	interrupted = false;
	
	if (rotating)
	{
		Serial.println("rotating!");
		for (k=0; k<STEPS_NUMBER; k++)
		{
			if (interrupted)
				break;
			
			delay(T/1000.);
			
			// ПЕРВЫЙ СТОЛБЕЦ ДИОДОВ
			if (k > starting_point[0] && k < (final_point[0] + 1))
			{
				digitalWrite(LATCH, LOW);
				int COMB = 0;
				for (int i=0; i<8; i++)
				{
					COMB += static_cast<int>( pow(2, i) + 0.5 )*POPA[i][k - starting_point[0] - 1];
				}				
				shiftOut(DATA, CLOCK, MSBFIRST, COMB); 
				digitalWrite(LATCH, HIGH);						
			}
			
			///* ВТОРОЙ СТОЛБЕЦ ДИОДОВ
			else if (k > starting_point[1] && k < (final_point[1] + 1))
			{
				digitalWrite(LATCH+3, LOW);
				int COMB = 0;
				for (int i=0; i<8; i++)
				{
					COMB += static_cast<int>( pow(2, i) + 0.5 )*POPA[i][k - starting_point[1] - 1];
				}				
				shiftOut(DATA+3, CLOCK+3, MSBFIRST, COMB); 
				digitalWrite(LATCH+3, HIGH);						
			}
			//*/
			else
			{
				digitalWrite(LATCH, LOW);
				shiftOut(DATA, CLOCK, MSBFIRST, 0); 
				digitalWrite(LATCH, HIGH);
				
				digitalWrite(LATCH+3, LOW);
				shiftOut(DATA+3, CLOCK+3, MSBFIRST, 0); 
				digitalWrite(LATCH+3, HIGH);
			}
		}
		if (!interrupted)rotating = false;
		Serial.println("LED blinking finished!");
	}
	else
	{		
		/*Serial.println("standby...");
		delay(3000);
		*/
	}	
}
	
// ******************************************************************************************