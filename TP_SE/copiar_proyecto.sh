#Este script copia un proyecto desde la Raspberry Pi a la PC usando SSH
#!/bin/bash

IP_RASPBERRY="192.168.1.100"  # Cambia por la IP real
USUARIO="nahuel"
RUTA_REMOTA="~/TP_SE/TP5_V1"
RUTA_LOCAL="~/Proyectos_Raspberry/"

echo "📡 Copiando proyecto desde Raspberry Pi..."
echo "IP: $IP_RASPBERRY"
echo "Usuario: $USUARIO"
echo "Carpeta: $RUTA_REMOTA"

# Crear directorio local si no existe
mkdir -p "$RUTA_LOCAL"

# Copiar via SCP
scp -r $USUARIO@$IP_RASPBERRY:$RUTA_REMOTA "$RUTA_LOCAL"

if [ $? -eq 0 ]; then
    echo "✅ Proyecto copiado exitosamente a: $RUTA_LOCAL"
    ls -la "$RUTA_LOCAL/TP5_V1/"
else
    echo "❌ Error al copiar el proyecto"
fi

