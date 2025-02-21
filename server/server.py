from flask import Flask, request, jsonify 
import speech_recognition as sr 
import base64
import os

server = Flask(__name__)

@server.route('/')
def index():
    return "Hello, World!"

@server.route('/test', methods=['POST'])
def test():
    print(request.json)
    return jsonify({'command': 'LED_ON'}), 200

@server.route('/speech', methods=['POST'])
def upload_audio():
    data = request.get_json(force=True)
    if not data or 'audio' not in data or 'content' not in data['audio']:
         return jsonify({'error': 'Nenhum aúdio enviado'}), 400

    audio_b64 = data['audio']['content']
    try:
        audio_data = base64.b64decode(audio_b64)
    except Exception as e:
        return jsonify({'error': 'Falha ao decodificar audio', 'message': str(e)}), 400

    file_path = 'audio.wav'
    with open(file_path, 'wb') as audio_f:
        audio_f.write(audio_data)

    if not os.path.exists(file_path):
        return jsonify({'error': 'Arquivo de audio não encontrado'}), 400

    recognizer = sr.Recognizer()
    try:
        with sr.AudioFile(file_path) as source:
            audio = recognizer.record(source)
        text = recognizer.recognize_google(audio, language='pt-BR')
        print(text)

        commands = {
            'acender': 'LED_ON',
            'apagar': 'LED_OFF'
        }
        action = commands.get(text.lower(), 'Comando não reconhecido')
        return jsonify({'command': action}), 200

    except sr.UnknownValueError:
        return jsonify({'error': 'Não foi possível reconhecer o áudio'}), 400
    except Exception as e:
        return jsonify({'error': 'Erro durante o processamento do áudio', 'message': str(e)}), 500

if __name__ == '__main__':
    server.run(host='0.0.0.0', port=5000)