# Esqueleto de Char Device con compilación en directorio build
FILE := char_device_3
AUTHOR := TD3
KDIR := /lib/modules/$(shell uname -r)/build
MDIR := $(PWD)/build

# Codigo fuente para compilar
obj-m += ${FILE}.o

# Compilacion completa en directorio build
all:
	@echo "🔨 Compilando esqueleto de char device..."
	@mkdir -p build;					\
	cp -f ${FILE}.c Makefile ${MDIR};	\
	cd build;									\
	make -C ${KDIR} M=${MDIR} modules
	@echo "✅ Compilación completada en directorio build/"

# Limpia el directorio de compilacion
clean:
	@echo "🧹 Limpiando directorio build..."
	@if [ -d "build" ]; then \
		cd build; \
		make -C ${KDIR} M=${MDIR} clean; \
		cd ..; \
		rm -rf build; \
	fi
	@echo "✅ Limpieza completada"

# Carga el modulo en el kernel
load:
	@echo "⬆️ Cargando módulo..."
	@sudo insmod build/${FILE}.ko

# Quita el modulo del kernel
unload:
	@echo "⬇️ Descargando módulo..."
	@sudo rmmod ${FILE}

# Muestra información del dispositivo
info:
	@echo "📊 Información del sistema:"
	@echo "=== ARCHIVO DISPOSITIVO ==="
	@ls -la /dev/td3_dispositivo 2>/dev/null || echo "El dispositivo no existe"
	@echo "=== MÓDULOS CARGADOS ==="
	@lsmod | grep td3 || echo "No hay módulos TD3 cargados"
	@echo "=== MENSAJES KERNEL ==="
	@dmesg | grep "TD3" | tail -5 || echo "No hay mensajes recientes"

# Prueba rápida del dispositivo
test:
	@echo "🧪 Probando dispositivo..."
	@echo "=== VERIFICANDO EXISTENCIA ==="
	@if [ -c "/dev/td3_dispositivo" ]; then \
		echo "✅ Archivo dispositivo EXISTE"; \
	else \
		echo "❌ Archivo dispositivo NO existe"; \
	fi
	@echo "=== INTENTO ESCRITURA ==="
	@echo "test" | sudo tee /dev/td3_dispositivo 2>&1 | grep -v "test" || true
	@echo "=== INTENTO LECTURA ==="
	@sudo cat /dev/td3_dispositivo 2>&1 | head -1 || true
	@echo "=== VERIFICANDO MENSAJES KERNEL ==="
	@dmesg | grep "TD3" | tail -3

# Limpia y recarga el módulo
reload: clean all load info

# Muestra ayuda
help:
	@echo "=== MAKE TARGETS PARA ESQUELETO CHAR DEVICE ==="
	@echo "all     : Compila el módulo en directorio build/ (por defecto)"
	@echo "clean   : Limpia el directorio build/"
	@echo "load    : Carga el módulo en el kernel"
	@echo "unload  : Descarga el módulo del kernel"
	@echo "info    : Muestra información del sistema"
	@echo "test    : Prueba básica del dispositivo"
	@echo "reload  : Limpia, recompila y recarga el módulo"
	@echo "help    : Muestra esta ayuda"