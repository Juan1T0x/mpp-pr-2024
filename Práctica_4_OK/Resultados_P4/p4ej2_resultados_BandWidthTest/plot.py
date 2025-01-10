import matplotlib.pyplot as plt
import pandas as pd

# Función para leer los datos de los archivos
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

# Leer los datos de los tres archivos para "pinned memory"
bandwidth_htod_pinned = read_bandwidth_data('HTOD_pinned.txt')
bandwidth_dtod_pinned = read_bandwidth_data('DTOD.txt')
bandwidth_dtoh_pinned = read_bandwidth_data('DTOH_pinned.txt')

# Leer los datos de los tres archivos para "pageable memory"
bandwidth_htod_pageable = read_bandwidth_data('HTOD__pageable.txt')
bandwidth_dtod_pageable = read_bandwidth_data('DTOD_pageable.txt')
bandwidth_dtoh_pageable = read_bandwidth_data('DTOH_pageable.txt')


# Crear la gráfica comparativa
plt.figure(figsize=(10, 6))

# HTOD
plt.plot(sizes, bandwidth_htod_pageable, label='HTOD (Pageable)', color='blue', marker='o')
plt.plot(sizes, bandwidth_htod_pinned, label='HTOD (Pinned)', color='cyan', marker='o', linestyle='--')

# DTOD
plt.plot(sizes, bandwidth_dtod_pageable, label='DTOD (Pageable)', color='green', marker='x')
plt.plot(sizes, bandwidth_dtod_pinned, label='DTOD (Pinned)', color='lime', marker='x', linestyle='--')

# DTOH
plt.plot(sizes, bandwidth_dtoh_pageable, label='DTOH (Pageable)', color='red', marker='s')
plt.plot(sizes, bandwidth_dtoh_pinned, label='DTOH (Pinned)', color='orange', marker='s', linestyle='--')

plt.xlabel('Tamaño de Bloque de Memoria (Bytes)')
plt.ylabel('Ancho de Banda (GB/s)')
plt.title('Ancho de Banda de Transferencias con Pageable vs Pinned Memory')
plt.legend()
plt.grid(True)
plt.xscale('log')
plt.yscale('log')

plt.savefig("bandwidth_comparator.png")

plt.show()

# Crear la tabla resumen con los valores medios de ancho de banda
data = {
    'Tipo de Transferencia': ['HTOD', 'DTOD', 'DTOH'],
    'Pageable Memory (GB/s)': [
        sum(bandwidth_htod_pageable) / len(bandwidth_htod_pageable),
        sum(bandwidth_dtod_pageable) / len(bandwidth_dtod_pageable),
        sum(bandwidth_dtoh_pageable) / len(bandwidth_dtoh_pageable)
    ],
    'Pinned Memory (GB/s)': [
        sum(bandwidth_htod_pinned) / len(bandwidth_htod_pinned),
        sum(bandwidth_dtod_pinned) / len(bandwidth_dtod_pinned),
        sum(bandwidth_dtoh_pinned) / len(bandwidth_dtoh_pinned)
    ]
}

df = pd.DataFrame(data)
import ace_tools as tools; tools.display_dataframe_to_user(name="Resumen de Ancho de Banda", dataframe=df)
