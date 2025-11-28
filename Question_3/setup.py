

from setuptools import setup, Extension

# Def C extension module
temp_stats_module = Extension(
    'temp_stats',                    
    sources=['temp_stats.c'],        
    include_dirs=[],                 
    libraries=[],                   
    extra_compile_args=['-O3'],     
)

# Setup configuration
setup(
    name='temp_stats',
    version='1.0',
    description='High-performance temperature statistics C extension for Python',
    author='Student',
    author_email='student@example.com',
    ext_modules=[temp_stats_module],
    
    # Metadata
    long_description='''
    This C extension provides efficient statistical operations on temperature data:
    - min_temp: Find minimum temperature
    - max_temp: Find maximum temperature  
    - avg_temp: Calculate average temperature
    - variance_temp: Calculate sample variance
    - count_readings: Count total readings
    
    Designed for real-time environmental monitoring systems requiring
    high-performance data processing.
    ''',
    
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Topic :: Scientific/Engineering',
        'Programming Language :: Python :: 3',
        'Programming Language :: C',
    ],
    
    python_requires='>=3.6',
)