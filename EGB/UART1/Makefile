obj-m += driver_serial.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
BUILD_DIR := $(PWD)/build
MODULE_NAME := driver_serial
DEVICE_NAME := /dev/pico_serial

all: | $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@mv *.ko *.mod.* *.o modules.order Module.symvers .tmp_versions $(BUILD_DIR)/ 2>/dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@rm -rf $(BUILD_DIR)

load: all
	@if lsmod | grep -q $(MODULE_NAME); then \
		echo "Módulo ya está cargado"; \
	else \
		sudo insmod $(BUILD_DIR)/$(MODULE_NAME).ko; \
		echo "Módulo cargado"; \
	fi

unload:
	@if lsmod | grep -q $(MODULE_NAME); then \
		sudo rmmod $(MODULE_NAME); \
		echo "Módulo descargado"; \
	else \
		echo "Módulo no estaba cargado"; \
	fi

reload: unload load
	@echo "Módulo recargado"

status:
	@if lsmod | grep -q $(MODULE_NAME); then \
		echo "Módulo $(MODULE_NAME) está cargado"; \
	else \
		echo "Módulo $(MODULE_NAME) NO está cargado"; \
	fi

# Comandos de prueba
test:
	@echo "=== Iniciando prueba del driver ==="
	@if ! lsmod | grep -q $(MODULE_NAME); then \
		echo "Cargando módulo..."; \
		$(MAKE) load; \
	fi
	sudo sh -c 'echo "LED_ON" > $(DEVICE_NAME)'
	@echo "LED encendido - esperando 2 segundos..."
	@sleep 2
	sudo sh -c 'echo "LED_OFF" > $(DEVICE_NAME)'
	@echo "LED apagado"
	@echo "=== Prueba completada ==="

test-on:
	@echo "Encendiendo LED..."
	sudo sh -c 'echo "LED_ON" > $(DEVICE_NAME)'

test-off:
	@echo "Apagando LED..."
	sudo sh -c 'echo "LED_OFF" > $(DEVICE_NAME)'

test-blink:
	@echo "=== Prueba de parpadeo ==="
	@if ! lsmod | grep -q $(MODULE_NAME); then \
		echo "Cargando módulo..."; \
		$(MAKE) load; \
	fi
	@for i in 1 2 3; do \
		echo "Ciclo $$i: ON"; \
		sudo sh -c 'echo "LED_ON" > $(DEVICE_NAME)'; \
		sleep 1; \
		echo "Ciclo $$i: OFF"; \
		sudo sh -c 'echo "LED_OFF" > $(DEVICE_NAME)'; \
		sleep 1; \
	done
	@echo "=== Prueba de parpadeo completada ==="

logs:
	@echo "=== Últimos logs del kernel ==="
	dmesg | tail -15 | grep -A 10 -B 5 "Pico Serial"

help:
	@echo "================================================"
	@echo "DRIVER UART PARA RASPBERRY PICO - COMANDOS"
	@echo "================================================"
	@echo ""
	@echo "COMPILACIÓN:"
	@echo "  make all     - Solo compilar el módulo"
	@echo "  make load    - Compilar y cargar el módulo"
	@echo "  make clean   - Limpiar archivos de compilación"
	@echo ""
	@echo "GESTIÓN DEL MÓDULO:"
	@echo "  make status  - Verificar si el módulo está cargado"
	@echo "  make unload  - Descargar el módulo del kernel"
	@echo "  make reload  - Recargar el módulo (unload + load)"
	@echo ""
	@echo "PRUEBAS:"
	@echo "  make test      - Prueba completa (ON + OFF automático)"
	@echo "  make test-on   - Encender LED"
	@echo "  make test-off  - Apagar LED"
	@echo "  make test-blink- Prueba de parpadeo (3 ciclos)"
	@echo ""
	@echo "DEBUG:"
	@echo "  make logs - Mostrar logs del kernel del driver"
	@echo ""
	@echo "FLUJO RECOMENDADO:"
	@echo "  1. make load      # Cargar módulo"
	@echo "  2. make test      # Probar funcionamiento"
	@echo "  3. make logs      # Verificar logs"
	@echo "  4. make unload    # Descargar módulo"
	@echo "================================================"
	
.PHONY: all clean load unload reload status test test-on test-off test-blink logs