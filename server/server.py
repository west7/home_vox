from flask import Flask, request, jsonify 
import speech_recognition as sr 
import base64
import os
import wave

server = Flask(__name__)

def create_wav_from_pcm(raw_pcm_data, output_filename, sample_rate=8000, num_channels=1, sample_width=2):
    """
    Cria um arquivo WAV válido a partir dos dados raw PCM.
    :param raw_pcm_data: Dados PCM brutos (bytes).
    :param output_filename: Nome do arquivo WAV a ser criado.
    :param sample_rate: Taxa de amostragem (ex.: 8000 Hz).
    :param num_channels: Número de canais (1 para mono, 2 para estéreo).
    :param sample_width: Largura da amostra em bytes (2 para 16 bits).
    """
    with wave.open(output_filename, 'wb') as wav_file:
        wav_file.setnchannels(num_channels)
        wav_file.setsampwidth(sample_width)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(raw_pcm_data)

@server.route('/speech', methods=['POST'])
def upload_audio():
    data = request.get_json(force=True)
    if not data or 'audio' not in data or 'content' not in data['audio']:
         return jsonify({'error': 'Nenhum áudio enviado'}), 400

    audio_b64 = data['audio']['content']
    try:
        # Decodifica os dados Base64 para obter os dados PCM brutos
        raw_pcm_data = base64.b64decode(audio_b64)
        print("Decoded PCM length:", len(raw_pcm_data))
    except Exception as e:
        return jsonify({'error': 'Falha ao decodificar áudio', 'message': str(e)}), 400

    # Cria o arquivo WAV a partir dos dados raw PCM
    wav_filename = 'audio.wav'
    create_wav_from_pcm(raw_pcm_data, wav_filename, sample_rate=8000, num_channels=1, sample_width=2)

    if not os.path.exists(wav_filename):
        return jsonify({'error': 'Arquivo de áudio não encontrado'}), 400

    # Aqui você pode usar sua biblioteca de reconhecimento de áudio, como o SpeechRecognition:
    import speech_recognition as sr
    recognizer = sr.Recognizer()
    try:
        with sr.AudioFile(wav_filename) as source:
            audio = recognizer.record(source)
        text = recognizer.recognize_google(audio, language='pt-BR')
        print(f"Texto reconhecido: {text}")

        commands = {
            'acender': 'LED_ON',
            'apagar': 'LED_OFF',
            'som': 'PLAY_SOUND',
            'parar': 'STOP_SOUND',
            'vermelho': 'SET_COLOR RED',
            'verde': 'SET_COLOR GREEN',
            'azul': 'SET_COLOR BLUE'
        }
        action = commands.get(text.lower(), 'Comando não reconhecido')
        return jsonify({'command': action}), 200

    except sr.UnknownValueError:
        return jsonify({'error': 'Não foi possível reconhecer o áudio'}), 400
    except Exception as e:
        return jsonify({'error': 'Erro durante o processamento do áudio', 'message': str(e)}), 500


if __name__ == '__main__':
    server.run(host='0.0.0.0', port=5000)