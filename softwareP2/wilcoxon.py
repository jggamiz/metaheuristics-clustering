import pandas as pd
from scipy.stats import wilcoxon

# Cargar los datos
df = pd.read_csv('det_results.csv')

# Filtrar solo el Genético Puro Base y el mejor Memético
df_filtered = df[df['Algorithm'].isin(['AGG-Uniform', 'AM-Best'])]

print("RESULTADOS DEL TEST DE WILCOXON (AGG-Uniform vs AM-Best)\n" + "-"*60)

# Agrupar por Dataset y k para hacer el test en cada escenario
escenarios = df_filtered.groupby(['Dataset', 'K'])

for (dataset, k), group in escenarios:
    # Extraer solo las columnas de semilla y fitness
    agg_df = group[group['Algorithm'] == 'AGG-Uniform'][['Seed', 'Fitness']]
    am_df = group[group['Algorithm'] == 'AM-Best'][['Seed', 'Fitness']]
    
    # Unir por semilla para asegurar que solo usamos pares idénticos
    merged = pd.merge(agg_df, am_df, on='Seed', suffixes=('_agg', '_am'))
    
    if len(merged) == 0:
        continue 

    # Aplicar el test de Wilcoxon (alternative='greater' porque queremos probar si AGG > AM, es decir, si AM es MEJOR/MENOR)
    stat, p_value = wilcoxon(merged['Fitness_agg'], merged['Fitness_am'], alternative='greater')
    
    # Mostrar resultados
    print(f"Dataset: {dataset:15} | k: {k} | p-valor: {p_value:.3e}")
    if p_value < 0.05:
        print("Diferencia significativa (AM-Best es estadísticamente mejor)\n")
    else:
        print("NO hay diferencia significativa\n")