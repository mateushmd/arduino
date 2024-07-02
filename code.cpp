#include <LiquidCrystal.h>

enum EstadosJogo
{
    INICIO = 0,
    SELECAO_JOGADORES = 1,
    COMECAR_RODADA = 2,
    RODADA_EM_ANDAMENTO = 3,
    FIM_RODADA = 4,
    FIM = 5
};

int estadoAtual = EstadosJogo::INICIO;

// Nº dos pinos dos componentes
const int BOTAO_CONTROLE = 9;
const int BUZZER = 10;
const int CONTROLE_LCD = 2;
const int JOGADORES[2] = {11, 12};

// Instância do Display LCD
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// Controla o estado do botão de começar/parar
bool pressionouControle = false;

// Controla o estado dos botões dos jogadores
bool pressionouJogadores[2] = {false, false};

// Armazena se os jogadores entraram no jogo
bool entrouJogadores[2] = {false, false};

// Placar
int pontuacao[2] = {0, 0};

// Contador de rodadas
int rodada = 0;

// Tempo para o buzzer tocar
unsigned long intervalo = 0;

// Marca um instante de tempo que, subtraindo do tempo atual, retorna o tempo passado
// Útil para tocar o buzzer no momento certo e verificar se os jogadores apertaram os botões antes ou depois do toque
unsigned long tempoAnterior;

// Se o tempo de espera acabou e os jogadores podem apertar o botão
bool liberado;

// Resultado da rodada
int resultado = -1;

void setup()
{
    pinMode(BOTAO_CONTROLE, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(CONTROLE_LCD, OUTPUT);
    digitalWrite(CONTROLE_LCD, HIGH); // Display LCD sempre ligado
    pinMode(JOGADORES[0], OUTPUT);
    pinMode(JOGADORES[1], OUTPUT);

    // Inicia o display
    lcd.begin(16, 2);

    // Gera uma seed nova toda vez que o jogo é iniciado para os números randômicos não se repetirem
    randomSeed(analogRead(0));

    atualizarEstadoJogo();
}

void loop()
{
    unsigned long tempoAtual = millis();

    // Se o botão foi apertado nesse exato momento
    if (digitalRead(BOTAO_CONTROLE) == HIGH && !pressionouControle)
    {
        pressionouControle = true;
        apertouControle();
    }
    else if (digitalRead(BOTAO_CONTROLE) == LOW)
    {
        pressionouControle = false;
    }

    // Se a rodada está acontecendo e chegou o momento de tocar o buzzer
    if (estadoAtual == EstadosJogo::RODADA_EM_ANDAMENTO &&
        !liberado && tempoAtual - tempoAnterior >= intervalo)
    {
        liberado = true;
        tone(BUZZER, 1000, 10);
    }

    apertouJogador(0);
    apertouJogador(1);

    if (estadoAtual == EstadosJogo::RODADA_EM_ANDAMENTO)
    {
        checarJogada();
    }
}

void apertouControle()
{
    if (estadoAtual == EstadosJogo::INICIO)
        estadoAtual = EstadosJogo::SELECAO_JOGADORES;
    else
        estadoAtual = EstadosJogo::FIM;

    atualizarEstadoJogo();
}

void apertouJogador(int jogador)
{
    if (digitalRead(JOGADORES[jogador]) == HIGH && !pressionouJogadores[jogador])
    {
        pressionouJogadores[jogador] = true;

        if (estadoAtual == EstadosJogo::SELECAO_JOGADORES)
        {
            entrouJogadores[jogador] = true;
            tone(BUZZER, 1000, 100);
            atualizarEstadoJogo();
        }
    }
    else if (digitalRead(JOGADORES[jogador]) == LOW)
    {
        pressionouJogadores[jogador] = false;
    }
}

void atualizarEstadoJogo()
{
    switch (estadoAtual)
    {
    case EstadosJogo::INICIO:
        reiniciarVariaveis();
        imprimirInicio();
        break;
    case EstadosJogo::SELECAO_JOGADORES:
        imprimirSelecaoJogadores();

        if (entrouJogadores[0] && entrouJogadores[1])
        {
            // Animação do Display quando ambos os jogadores entram
            for (int i = 0; i < 3; i++)
            {
                lcd.clear();
                delay(500);
                imprimirSelecaoJogadores();
                delay(500);
            }
            delay(1500);

            estadoAtual = EstadosJogo::COMECAR_RODADA;
            atualizarEstadoJogo();
        }
        break;
    case EstadosJogo::COMECAR_RODADA:
        comecarRodada();
        break;
    case EstadosJogo::RODADA_EM_ANDAMENTO:
        tempoAnterior = millis();
        imprimirPlacar();
        break;
    case EstadosJogo::FIM_RODADA:
        if (resultado > -1)
            pontuacao[resultado]++;

        imprimirResultado(resultado);

        estadoAtual = EstadosJogo::COMECAR_RODADA;
        atualizarEstadoJogo();
        break;
    case EstadosJogo::FIM:
        resultadoFinal();
        break;
    }
}

void comecarRodada()
{
    // Considerando que os jogadores ainda não pressionaram
    pressionouJogadores[0] = false;
    pressionouJogadores[1] = false;

    lcd.clear();

    String rodadaStr = "RODADA ";
    rodadaStr += ++rodada;

    // Ajustando a posição do cursor no display para imprimir o texto no centro
    int coluna = (16 - rodadaStr.length()) / 2;

    printLcd(coluna, 0, rodadaStr);

    delay(1000);

    estadoAtual = EstadosJogo::RODADA_EM_ANDAMENTO;
    intervalo = random(1000, 2001);

    liberado = false;

    atualizarEstadoJogo();
}

void checarJogada()
{
    if (pressionouJogadores[0] || pressionouJogadores[1])
    {
        estadoAtual = EstadosJogo::FIM_RODADA;

        if (pressionouJogadores[0] && pressionouJogadores[1]) // Empate
            resultado = -1;
        else
        {
            for (int i = 0; i < 2; i++)
            {
                if (pressionouJogadores[i])
                {
                    if (liberado)
                        resultado = i;
                    else
                        resultado = !i;
                }
            }
        }

        atualizarEstadoJogo();
    }
}

void resultadoFinal()
{
    // Se algum jogador pontuou antes de parar o jogo
    if (pontuacao[0] > 0 || pontuacao[1] > 0)
    {
        int vencedor;

        if (pontuacao[0] == pontuacao[1])
            vencedor = -1;
        else if (pontuacao[0] > pontuacao[1])
            vencedor = 0;
        else
            vencedor = 1;

        imprimirResultado(vencedor);
    }

    estadoAtual = EstadosJogo::INICIO;
    atualizarEstadoJogo();
}

void reiniciarVariaveis()
{
    for (int i = 0; i < 2; i++)
    {
        pressionouJogadores[i] = false;
        entrouJogadores[i] = false;
        pontuacao[i] = 0;
    }

    intervalo = 0;

    liberado = false;

    rodada = 0;

    resultado = -1;
}

//--------------------------------------
//		FUNÇÕES PARA IMPRIMIR NO DISPLAY
//--------------------------------------

void printLcd(int c, int l, String str)
{
    lcd.setCursor(c, l);
    lcd.print(str);
}

void imprimirInicio()
{
    lcd.clear();

    printLcd(2, 0, "BOTAO CENTRAL");
    printLcd(2, 1, "PARA COMECAR");
}

void imprimirSelecaoJogadores()
{
    lcd.clear();

    if (entrouJogadores[0])
        printLcd(0, 0, "J1 ENTROU");
    else
        printLcd(0, 0, "J1 ESPERANDO...");

    if (entrouJogadores[1])
        printLcd(0, 1, "J2 ENTROU");
    else
        printLcd(0, 1, "J2 ESPERANDO...");
}

void imprimirPlacar()
{
    lcd.clear();

    String pontuacaoJ1 = "J1 - ";
    pontuacaoJ1 += pontuacao[0];
    printLcd(0, 0, pontuacaoJ1);

    String valor = String(pontuacao[1]);
    String pontuacaoJ2 = valor + " - J2";

    // Posicionando a pontuação do jogador 2 no canto direito do display
    printLcd(16 - pontuacaoJ2.length(), 1, pontuacaoJ2);
}

void imprimirResultado(int resultado)
{
    lcd.clear();

    if (resultado == -1)
        printLcd(5, 0, "EMPATE");
    else
    {
        String strResultado = String(resultado + 1);
        printLcd(3, 0, "JOGADOR " + strResultado);
        printLcd(5, 1, "VENCEU");
    }

    delay(2000);
}