from flask import Flask, render_template, redirect, url_for, request, jsonify
from datetime import datetime
import os
import logging

app = Flask(__name__)

# Variables globales
setpoint = 15.0
altura_maxima = 20.0
altura_minima = 10.0

# Archivos de logs y estado
ruta_historial = "comandos_logs.csv"
ruta_estado = "current_state.csv"
ruta_conexiones = "flask_connections.log"

# === CONFIGURAR LOGGING DE FLASK ===
logging.basicConfig(
    filename=ruta_conexiones,
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
)
log = logging.getLogger()

# Crear encabezado de CSV si no existe
if not os.path.exists(ruta_historial):
    with open(ruta_historial, "w") as f:
        f.write("timestamp,setpoint,altura_maxima,altura_minima\n")

@app.route('/')
def index():
    log.info("Acceso a la página principal")
    return render_template('index.html', 
                         setpoint=setpoint, 
                         altura_maxima=altura_maxima, 
                         altura_minima=altura_minima)

@app.route('/configurar', methods=['POST'])
def configurar():
    global setpoint, altura_maxima, altura_minima
    
    try:
        # Obtener datos del formulario
        setpoint = float(request.form.get('setpoint'))
        altura_maxima = float(request.form.get('altura_maxima'))
        altura_minima = float(request.form.get('altura_minima'))
        
        # Validaciones
        if altura_minima >= altura_maxima:
            return jsonify({'error': 'La altura mínima debe ser menor que la altura máxima'}), 400
        
        if setpoint < altura_minima or setpoint > altura_maxima:
            return jsonify({'error': 'El setpoint debe estar entre la altura mínima y máxima'}), 400
        
        # Guardar en logs
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(ruta_historial, "a") as f:
            f.write(f"{timestamp},{setpoint},{altura_maxima},{altura_minima}\n")
        
        # Guardar estado actual
        with open(ruta_estado, "w") as f:
            f.write(f"setpoint={setpoint}, altura_maxima={altura_maxima}, altura_minima={altura_minima}")
        
        log.info(f"Configuración guardada: setpoint={setpoint}, max={altura_maxima}, min={altura_minima}")
        
        return jsonify({
            'success': True,
            'setpoint': setpoint,
            'altura_maxima': altura_maxima,
            'altura_minima': altura_minima
        })
    
    except ValueError as e:
        return jsonify({'error': 'Valores numéricos inválidos'}), 400
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/reset', methods=['POST'])
def reset():
    global setpoint, altura_maxima, altura_minima
    setpoint = 15.0
    altura_maxima = 20.0
    altura_minima = 10.0
    
    log.info("Valores restablecidos a valores por defecto")
    return jsonify({
        'success': True,
        'setpoint': setpoint,
        'altura_maxima': altura_maxima,
        'altura_minima': altura_minima
    })

# === VER LOGS DE COMANDOS ===
@app.route('/ver_logs')
def ver_logs():
    if not os.path.exists(ruta_historial):
        return "<h3>Sin registros todavía.</h3>"

    with open(ruta_historial, "r") as f:
        lineas = f.readlines()

    html = """
    <html><head>
    <title>Logs de Configuraciones</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f8f8f8; padding: 2rem; }
        table { border-collapse: collapse; width: 100%; background: white; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        th, td { border: 1px solid #ddd; padding: 12px 15px; text-align: left; }
        th { background-color: #4CAF50; color: white; }
        tr:nth-child(even) { background-color: #f2f2f2; }
        tr:hover { background-color: #e9f5e9; }
        h2 { color: #333; margin-bottom: 20px; }
    </style></head><body>
    <h2>📜 Registro de Configuraciones</h2>
    <table>
    <tr><th>Timestamp</th><th>Setpoint</th><th>Altura Máxima</th><th>Altura Mínima</th></tr>
    """
    for linea in lineas[1:]:
        campos = linea.strip().split(',')
        if len(campos) == 4:
            html += f"<tr><td>{campos[0]}</td><td>{campos[1]} m</td><td>{campos[2]} m</td><td>{campos[3]} m</td></tr>"
    html += "</table><br><a href='/'>Volver a la configuración</a></body></html>"
    return html

# === VER CONEXIONES DE FLASK ===
@app.route('/ver_conexiones')
def ver_conexiones():
    if not os.path.exists(ruta_conexiones):
        return "<h3>Sin conexiones registradas todavía.</h3>"

    with open(ruta_conexiones, "r") as f:
        contenido = f.read().splitlines()

    html = """
    <html><head><title>Conexiones Flask</title>
    <style>
        body { background-color: #111; color: #0f0; font-family: monospace; padding: 2rem; }
        h2 { color: #0ff; }
        pre { white-space: pre-wrap; background: #000; padding: 1rem; border-radius: 5px; }
        a { color: #0ff; text-decoration: none; }
        a:hover { text-decoration: underline; }
    </style></head><body>
    <h2>🔌 Conexiones y actividad Flask</h2>
    <pre>
    """ + "\n".join(contenido[-100:]) + """
    </pre>
    <br>
    <a href='/'>Volver a la configuración</a>
    </body></html>"""
    return html

@app.route('/estado_actual')
def estado_actual():
    return jsonify({
        'setpoint': setpoint,
        'altura_maxima': altura_maxima,
        'altura_minima': altura_minima
    })

if __name__ == '__main__':
    log.info("Servidor Flask iniciado - Sistema de Configuración de Parámetros")
    app.run(host='0.0.0.0', port=5000, debug=True)