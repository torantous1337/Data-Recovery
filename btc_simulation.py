import numpy as np
import pandas as pd

def main():
    # Parameters for Geometric Brownian Motion
    np.random.seed(42)  # For reproducible results, adjust or remove for different runs
    S0 = 60000.0        # Initial Bitcoin price
    mu = 0.5            # Expected annual return
    sigma = 0.8         # Expected annual volatility
    days = 60
    dt = 1.0 / 365.0

    # Generate 60 days of prices using GBM
    prices = [S0]
    for _ in range(1, days):
        Z = np.random.standard_normal()
        S_t = prices[-1] * np.exp((mu - 0.5 * sigma**2) * dt + sigma * np.sqrt(dt) * Z)
        prices.append(S_t)

    # Create DataFrame
    df = pd.DataFrame({'Day': range(1, days+1), 'Price': prices})

    # Calculate Moving Averages
    df['7_MA'] = df['Price'].rolling(window=7).mean()
    df['30_MA'] = df['Price'].rolling(window=30).mean()

    # Trading simulation
    initial_fiat = 10000.0
    fiat = initial_fiat
    btc = 0.0

    print("=== Daily Ledger ===")
    for i in range(days):
        day = df.loc[i, 'Day']
        price = df.loc[i, 'Price']
        ma7 = df.loc[i, '7_MA']
        ma30 = df.loc[i, '30_MA']

        action = "HOLD"

        # Check for valid MAs
        if pd.notna(ma7) and pd.notna(ma30):
            if i > 0 and pd.notna(df.loc[i-1, '7_MA']) and pd.notna(df.loc[i-1, '30_MA']):
                prev_ma7 = df.loc[i-1, '7_MA']
                prev_ma30 = df.loc[i-1, '30_MA']

                # Buy condition: 7_MA crosses above 30_MA (Golden Cross)
                if (prev_ma7 <= prev_ma30) and (ma7 > ma30):
                    if fiat > 0:
                        btc_bought = fiat / price
                        btc += btc_bought
                        fiat = 0.0
                        action = f"BUY {btc_bought:.6f} BTC"

                # Sell condition: 7_MA crosses below 30_MA (Death Cross)
                elif (prev_ma7 >= prev_ma30) and (ma7 < ma30):
                    if btc > 0:
                        fiat_received = btc * price
                        fiat += fiat_received
                        btc = 0.0
                        action = f"SELL all BTC for ${fiat_received:.2f}"

        # Format output safely with NaN handling
        ma7_str = f"${ma7:8.2f}" if pd.notna(ma7) else "     N/A"
        ma30_str = f"${ma30:8.2f}" if pd.notna(ma30) else "     N/A"
        print(f"Day {day:2d} | Price: ${price:8.2f} | 7-MA: {ma7_str} | 30-MA: {ma30_str} | Action: {action}")

    # Final Portfolio Calculation
    final_portfolio_value = fiat + (btc * df.iloc[-1]['Price'])
    total_return = ((final_portfolio_value - initial_fiat) / initial_fiat) * 100

    print("\n=== Final Portfolio Performance ===")
    print(f"Initial Fiat     : ${initial_fiat:.2f}")
    print(f"Final Fiat       : ${fiat:.2f}")
    print(f"Final BTC        : {btc:.6f}")
    print(f"Final Total Value: ${final_portfolio_value:.2f}")
    print(f"Total Return     : {total_return:.2f}%")

if __name__ == "__main__":
    main()
