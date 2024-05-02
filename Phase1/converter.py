import pandas as pd
import matplotlib.pyplot as plt

def read_text_file(file_path):
    """
    Read a text file and return its lines.
    Args:
        file_path (str): Path to the text file.
    Returns:
        list: List of lines from the text file.
    """
    with open(file_path, 'r') as file:
        lines = file.readlines()
    return lines

def create_dataframe_from_perf_file(file_path):
    """
    Create a DataFrame from a performance text file.
    Args:
        file_path (str): Path to the performance text file.
    Returns:
        pandas.DataFrame: DataFrame created from the text file.
    """
    lines = read_text_file(file_path)
    data = [line.strip().split('\t') for line in lines]
    df = pd.DataFrame(data, columns=['Metric', 'Value'])
    df['Value'] = pd.to_numeric(df['Value'])
    return df

def create_dataframe_from_log_file(file_path):
    """
    Create a DataFrame from a log text file.
    Args:
        file_path (str): Path to the log text file.
    Returns:
        pandas.DataFrame: DataFrame created from the text file.
    """
    lines = read_text_file(file_path)
    data = []
    for line in lines:
        parts = line.strip().split('\t')
        new_parts = []
        for part in parts:
            if ':' in part:
                new_parts.extend(part.split(':'))
            else:
                new_parts.append(part)
        data.append(new_parts)

    max_cols = max(len(parts) for parts in data)
    columns = [f'Column_{i+1}' for i in range(max_cols)]
    df = pd.DataFrame(data, columns=columns)

    numeric_cols = ['Column_1', 'Column_2', 'Column_4', 'Column_6', 'Column_8', 'Column_10', 'Column_12', 'Column_14']
    for col in df.columns:
        if col in numeric_cols:
            df[col] = pd.to_numeric(df[col], errors='ignore')

    return df

def plot_dataframe_as_table(df, image_path):
    """
    Plot a DataFrame as a table and save it as an image file.
    Args:
        df (pandas.DataFrame): DataFrame to be plotted.
        image_path (str): Path to save the image file.
    """
    fig, ax = plt.subplots(figsize=(6, 4))
    ax.axis('tight')
    ax.axis('off')
    table = ax.table(cellText=df.values, colLabels=df.columns, loc='center')
    plt.savefig(image_path, bbox_inches='tight', pad_inches=0.1)
    plt.show()

if __name__ == "__main__":
    # File paths
    perf_file_path = 'Phase1/outputs/scheduler.perf'
    log_file_path = 'Phase1/outputs/scheduler.log'

    # Process performance file
    perf_df = create_dataframe_from_perf_file(perf_file_path)
    plot_dataframe_as_table(perf_df, 'perf_df_to_image.png')

    # Process log file
    log_df = create_dataframe_from_log_file(log_file_path)
    plot_dataframe_as_table(log_df, 'log_df_to_image.png')
