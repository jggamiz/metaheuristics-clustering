import pandas as pd
from scipy.stats import wilcoxon

# Asegúrate de cargar el archivo con los resultados detallados por semilla (no el de medias)
df = pd.read_csv('det_results.csv')

# Limpiar espacios por si acaso
df['Algorithm'] = df['Algorithm'].astype(str).str.strip()

# Nombres exactos extraídos de tu CSV
comparaciones = [
    ('LocalSearch', 'SimulatedAnnealing'),
    ('LocalSearch', 'ILS'),
    ('BMB', 'GRASP'),
    ('ILS', 'AM-Best')
]

escenarios = df.groupby(['Dataset', 'K'])

for alg_base, alg_mejora in comparaciones:
    print(f"\n{'='*60}\nTEST DE WILCOXON: {alg_base} vs {alg_mejora}\n{'='*60}")
    
    for (dataset, k), group in escenarios:
        # Extraer solo las columnas de semilla y fitness (asegúrate de que estas columnas se llaman así en tu CSV detallado)
        base_df = group[group['Algorithm'] == alg_base][['Seed', 'Fitness']]
        mejora_df = group[group['Algorithm'] == alg_mejora][['Seed', 'Fitness']]
        
        merged = pd.merge(base_df, mejora_df, on='Seed', suffixes=('_base', '_mejora'))
        
        if len(merged) < 10:
            print(f"Dataset: {dataset:15} | k: {k} | Error: Faltan semillas (Encontradas: {len(merged)})")
            continue 
            
        try:
            stat, p_value = wilcoxon(merged['Fitness_base'], merged['Fitness_mejora'], alternative='greater')
            
            if p_value < 0.05:
                resultado = f"Gana {alg_mejora} (Significativo)"
            elif p_value > 0.95:
                resultado = f"Gana {alg_base} (Significativo)"
            else:
                resultado = "Empate técnico (No significativo)"
                
            print(f"Dataset: {dataset:15} | k: {k:2} | p-valor: {p_value:.3e} | {resultado}")
            
        except ValueError:
             print(f"Dataset: {dataset:15} | k: {k:2} | Test no aplicable (diferencias nulas exactas)")