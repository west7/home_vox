from flask import Flask, request, jsonify 
import speech_recognition as sr 


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
    if 'audio' not in request.files:
        return jsonify({'error': 'Nenhum arquivo enviado'}), 400
    
    audio_file = request.files['audio']
    file_path = "audio.wav"
    audio_file.save(file_path)

    recognize = sr.Recognizer()
    with sr.AudioFile(file_path) as source:
        audio = recognize.record(source)

    try:
        text = recognize.recognize_google(audio, language='pt-BR')
        print(f'Texto reconhecido: {text}')

        commands = {
            'acender led': 'LED_ON',
            'apagar led': 'LED_OFF'
        }

        action = commands.get(text.lower(), "Comando não reconhecido")
        return jsonify({'command': action}), 200
    
    except sr.UnknownValueError:
        return jsonify({'error': 'Não foi possível reconhecer o áudio'}), 400


if __name__ == '__main__':
    server.run(host='0.0.0.0', port=5000)