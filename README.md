# Home Vox

**Home Vox** é um assitente virtual inteligente desenvolvido para automação de tarefas e controle de dispositivos eletrônicos. Construído usando a [BitDogLab](https://github.com/BitDogLab/BitDogLab/tree/main), ele escuta comandos de voz e executa ações de acordo com o que foi solicitado.

## Funcionalidades

- Reconhecimento de voz: por enquanto, suporta comandos básicos:
  - `LED_OFF`: Desliga os LEDs
  - `SET_COLOR RED`: Define a cor do LED como vermelho
  - `SET_COLOR GREEN`: Define a cor do LED como verde
  - `SET_COLOR BLUE`: Define a cor do LED como azul
- Conectividade Wi-Fi: Conecta-se a uma rede Wi-Fi para processar comandos de voz
- Processamento em tempo real: Utiliza DMA e ADC para processar o sinal de áudio em tempo real
- Integração com API: Se comunica com uma API externa para processar os comandos de voz

## Hardware

Utiliza os seguintes componentes e recursos da plataforma BitDogLab:
- Módulo Wi-Fi da Pico W
- Microfone com amplificador (MAX4466EXK)
- LED RGB
- Botão para controle de gravação de voz
- DMA e ADC para processamento de áudio
- Comunicação com a API via TCP/IP
- Comunicação serial para depuração

## Software

- Linguagem de programação: C (Raspberry Pi Pico SDK)
- CMake

## Setup

1. Clone o repositório:
```bash
git clone https://github.com/west7/home_vox.git
```
2. Compile o código:
```bash
cd home_vox
mkdir build
cd build
cmake ..
make
```
3. Conecte o microcontrolador à porta USB do computador e faço o upload do firmware

4. Suba a API
```bash
cd server
source env/bin/activate
python3 server.py
```
## Uso

1. Espere as configurações de rede serem carregadas e o Home Vox estar pronto para uso 
2. Garanta que as credenciais de rede estão corretas no arquivo `wifi.h`
3. Fala o comando de voz suportado de maneira clara
4. Observe o feedback nos LEDs RGB

## Planos Futuros

- Adicionar suporte a mais comandos de voz
- Adicionar suporte a mais dispositivos eletrônicos
- Implementar um aplicativo móvel para controle remoto

