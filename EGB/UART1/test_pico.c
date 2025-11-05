#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;
    const char *device_path = "/dev/pico_serial";
    
    if (argc != 2) {
        printf("Uso: %s <LED_ON|LED_OFF>\n", argv[0]);
        return 1;
    }
    
    fd = open(device_path, O_WRONLY);
    if (fd < 0) {
        perror("Error abriendo dispositivo");
        return 1;
    }
    
    if (write(fd, argv[1], strlen(argv[1])) < 0) {
        perror("Error escribiendo en dispositivo");
        close(fd);
        return 1;
    }
    
    printf("Comando enviado: %s\n", argv[1]);
    close(fd);
    return 0;
}