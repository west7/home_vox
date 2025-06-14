# Home Vox

> **Home Vox** é um assitente virtual inteligente desenvolvido para automação de tarefas e controle de dispositivos eletrônicos. Construído usando a [BitDogLab](https://github.com/BitDogLab/BitDogLab/tree/main), ele escuta comandos de voz e executa ações de acordo com o que foi solicitado.

## Objetivo 🎯

O objetivo do projeto é criar um assistente virtual que possa ser utilizado para controlar dispositivos eletrônicos em casa. O Home Vox é capaz de reconhecer comandos de voz e executar ações de acordo com o que foi solicitado. Por exemplo, ele poderá ser utilizado para ligar e desligar lâmpadas, controlar a temperatura do ar condicionado, abrir e fechar cortinas, entre outras tarefas.

## Vídeo

<div align="center">

[![Vídeo](https://img.youtube.com/vi/Q1LQnpzrpS8/hqdefault.jpg)](https://www.youtube.com/watch?v=Q1LQnpzrpS8)

</div>

## Funcionalidades 🚀

- Reconhecimento de voz: por enquanto, suporta comandos básicos:
  - `Apagar` Desliga os LEDs
  - `Vermelho` Define a cor do LED como vermelho
  - `Verde` Define a cor do LED como verde
  - `Azul`  Define a cor do LED como azul
- Conectividade Wi-Fi: Conecta-se a uma rede Wi-Fi para processar comandos de voz
- Processamento em tempo real: Utiliza DMA e ADC para processar o sinal de áudio em tempo real
- Integração com API: Se comunica com uma API externa para processar os comandos de voz

## Hardware 🛠️

Utiliza os seguintes componentes e recursos da plataforma BitDogLab:
- Módulo Wi-Fi da Pico W
- Microfone com amplificador (MAX4466EXK)
- LED RGB
- Botão para controle de gravação de voz
- Display OLED para feedback visual
- DMA e ADC para processamento de áudio
- Comunicação serial para depuração

## Software 🖥️

- Linguagem de programação: C (Raspberry Pi Pico SDK)
- CMake

## Setup 🔧

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
python3 -m venv env
source env/bin/activate # Linux e MacOS
env\Scripts\activate # Windows
pip install -r requirements.txt
python3 server.py
```

## Uso ⚡

1. Espere as configurações de rede serem carregadas e o Home Vox estar pronto para uso 
2. Garanta que as credenciais de rede estão corretas no arquivo `wifi.h`
3. Fale o comando de voz suportado de maneira clara
4. Observe o feedback nos LEDs RGB

## Planos Futuros

- Adicionar suporte a mais comandos de voz
- Adicionar suporte a mais dispositivos eletrônicos
- Implementar um aplicativo móvel para controle remoto

