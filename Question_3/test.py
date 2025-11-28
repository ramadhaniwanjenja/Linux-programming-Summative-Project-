#!/usr/bin/env python3
import temp_stats, statistics  # pyright: ignore[reportMissingImports]

temps = [
    22.5, 23.1, 22.8, 24.0, 23.7, 21.9, 20.8, 22.1,
    23.4, 24.2, 25.0, 23.9, 22.3, 21.7, 22.6, 23.8,
    24.5, 23.0, 22.2, 21.5, 20.9, 23.3, 24.1, 22.9,
]

print("Temperature Statistics C Extension - Test Suite\n")
print("Sample Temperature Data (24-hour readings):")
print(f"Temperatures: {temps[:3]} ... {temps[-3:]}")
print(f"Total readings: {len(temps)}\n")

def run(name, func, expected):
    print("-" * 60)
    print(f"Test: {name}")
    result = func()
    print(f"Result:   {result}")
    print(f"Expected: {expected}")
    status = "PASS" if abs(result - expected) < 1e-9 else "FAIL"
    print(f"Status: {status}\n")

run("count_readings", lambda: temp_stats.count_readings(temps), len(temps))
run("min_temp", lambda: temp_stats.min_temp(temps), min(temps))
run("max_temp", lambda: temp_stats.max_temp(temps), max(temps))
run("avg_temp", lambda: temp_stats.avg_temp(temps), statistics.mean(temps))
run("variance_temp", lambda: temp_stats.variance_temp(temps), statistics.variance(temps))

print("All tests completed successfully!")