import os
import pandas as pd

# Buscamos archivos que empiecen por 'out_' y terminen en '.txt'
result_files = [f for f in os.listdir('.') if f.startswith('out_') and f.endswith('.txt')]

data = []

for file in result_files:
    try:
        # Parseo del nombre de archivo:
        # Formato: out_<n>_<m>_<n_gen>_<tam_pob>_<np>_<ngm>.txt
        parts = file.split('_')
        n = int(parts[1])                          # <n>
        m = int(parts[2])                          # <m>
        n_gen = int(parts[3])                      # <n_gen>
        tam_pob = int(parts[4])                    # <tam_pob>
        np = int(parts[5])                         # <np>
        ngm = int(parts[6].split('.')[0])          # <ngm> (quitamos '.txt')

        exe_time = None

        # Leemos el archivo para extraer "Execution Time"
        with open(file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if "Execution Time" in line:
                    # Formato: "Execution Time: 13.37 sec"
                    parts_line = line.split(':')
                    if len(parts_line) >= 2:
                        valor_str = parts_line[1].replace('sec', '').strip()
                        exe_time = float(valor_str)

        # Si encontramos el exe_time, guardamos los datos
        if exe_time is not None:
            data.append({
                'n': n,
                'm': m,
                'n_gen': n_gen,
                'tam_pob': tam_pob,
                'np': np,
                'ngm': ngm,
                'exe_time': exe_time
            })
        else:
            print(f"No se encontró 'Execution Time' en el archivo: {file}")
    except Exception as e:
        print(f"Error procesando el archivo {file}: {e}")

# Convertimos la lista de diccionarios a DataFrame
df = pd.DataFrame(data)

if not df.empty:
    # Aseguramos un orden en las columnas
    df = df[['n', 'm', 'n_gen', 'tam_pob', 'np', 'ngm', 'exe_time']]

    # Exportamos a Excel
    output_file = "resultados.xlsx"
    df.to_excel(output_file, index=False)
    print(f"Datos exportados a '{output_file}'")
else:
    print("No se encontraron datos válidos para exportar.")
