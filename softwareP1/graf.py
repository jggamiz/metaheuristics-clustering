import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

def main():
    fname = "det_results.csv"
    output_dir = "graficas"
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    if not os.path.exists(fname):
        print(f"Error: No se encuentra el archivo {fname} en esta ruta.")
        return

    df = pd.read_csv(fname)
    
    df['Algorithm'] = df['Algorithm'].replace({
        'RandomSearch': 'RANDOM',
        'GreedySearch': 'GREEDY',
        'LocalSearch': 'LOCAL'
    })

    datasets = df['Dataset'].unique()
    ks = df['K'].unique()

    for dataset in datasets:
        for k in ks:
            subset = df[(df['Dataset'] == dataset) & (df['K'] == k)]
            
            if subset.empty:
                continue
                
            nombre_limpio = dataset.split('/')[-1] 
            
            # --- GRÁFICA 1: TODOS LOS ALGORITMOS ---
            plt.figure(figsize=(6, 5))
            p1 = sns.boxplot(data=subset, x="Algorithm", y="Fitness")
            p1.set_title(f"{nombre_limpio} (k={k}) - Todos")
            p1.set_xlabel("Algoritmo")
            p1.set_ylabel("Fitness")
            
            filename_all = f"boxplot_{nombre_limpio}_k{k}_all.png"
            filepath_all = os.path.join(output_dir, filename_all)
            plt.tight_layout()
            plt.savefig(filepath_all)
            plt.close()
            print(f"Generado: {filepath_all}")

            # --- GRÁFICA 2: ZOOM (SOLO RANDOM VS LOCAL) ---
            # Filtramos fuera el GREEDY para ver la diferencia real entre las metaheurísticas
            subset_meta = subset[subset['Algorithm'] != 'GREEDY']
            
            plt.figure(figsize=(6, 5))
            p2 = sns.boxplot(data=subset_meta, x="Algorithm", y="Fitness")
            p2.set_title(f"{nombre_limpio} (k={k}) - Random vs Local")
            p2.set_xlabel("Algoritmo")
            p2.set_ylabel("Fitness")
            
            filename_zoom = f"boxplot_{nombre_limpio}_k{k}_zoom.png"
            filepath_zoom = os.path.join(output_dir, filename_zoom)
            plt.tight_layout()
            plt.savefig(filepath_zoom)
            plt.close()
            print(f"Generado: {filepath_zoom}")

if __name__ == '__main__':
    main()
