import matplotlib.pyplot as plt

# Función para leer los datos de un archivo
def read_bandwidth_data(file_path):
    sizes = []
    bandwidth = []
    with open(file_path, 'r') as file:
        for line in file.readlines():
            parts = line.split()
            if len(parts) == 2:
                sizes.append(int(parts[0]))  # Tamaño en bytes
                bandwidth.append(float(parts[1]))  # Ancho de banda en GB/s
    return bandwidth

sizes = [
    1024, 525312, 1049600, 1573888, 2098176, 2622464, 3146752, 3671040, 4195328, 4719616,
    5243904, 5768192, 6292480, 6816768, 7341056, 7865344, 8389632, 8913920, 9438208, 9962496,
    10486784, 11011072, 11535360, 12059648, 12583936, 13108224, 13632512, 14156800, 14681088, 15205376,
    15729664, 16253952, 16778240, 17302528, 17826816, 18351104, 18875392, 19399680, 19923968, 20448256,
    20972544, 21496832, 22021120, 22545408, 23069696, 23593984, 24118272, 24642560, 25166848, 25691136,
    26215424, 26739712, 27264000, 27788288, 28312576, 28836864, 29361152, 29885440, 30409728, 30934016,
    31458304, 31982592, 32506880, 33031168, 33555456, 34079744, 34604032, 35128320, 35652608, 36176896,
    36701184, 37225472, 37749760, 38274048, 38798336, 39322624, 39846912, 40371200, 40895488, 41419776,
    41944064, 42468352, 42992640, 43516928, 44041216, 44565504, 45089792, 45614080, 46138368, 46662656,
    47186944, 47711232, 48235520, 48759808, 49284096, 49808384, 50332672, 50856960, 51381248, 51905536,
    52429824, 52954112, 53478400, 54002688, 54526976, 55051264, 55575552, 56099840, 56624128, 57148416,
    57672704, 58196992, 58721280, 59245568, 59769856, 60294144, 60818432, 61342720, 61867008, 62391296,
    62915584, 63439872, 63964160, 64488448, 65012736, 65537024, 66061312, 66585600
]

# Leer los datos de los tres archivos
bandwidth_htod_pageable = read_bandwidth_data('HTOD__pageable.txt')
bandwidth_dtod_pageable = read_bandwidth_data('DTOD_pageable.txt')
bandwidth_dtoh_pageable = read_bandwidth_data('DTOH_pageable.txt')

# Verificar que las longitudes de las listas coinciden y ajustar
print(f"Tamaño de HTOD: {len(sizes)} muestras")
print(f"Tamaño de DTOD: {len(sizes)} muestras")
print(f"Tamaño de DTOH: {len(sizes)} muestras")

# Crear el gráfico
plt.figure(figsize=(10, 6))
plt.plot(sizes, bandwidth_htod, label='Host a dispositivo', color='blue', marker='o')
plt.plot(sizes, bandwidth_dtod, label='Dentro de dispositivo', color='green', marker='x')
plt.plot(sizes, bandwidth_dtoh, label='Dispositivo a host', color='red', marker='s')

# Configurar las etiquetas y título
plt.xlabel('Tamaño de Bloque de Memoria (Bytes)')
plt.ylabel('Ancho de Banda (GB/s)')
plt.title('Ancho de Banda de Transferencias con Pageable Memory')
plt.legend()
plt.grid(True)

# Usar escala logarítmica para mejor visualización
plt.xscale('log')
plt.yscale('log')

# Mostrar el gráfico
plt.savefig("bandwidth_plot_pageable.png")

plt.show()
