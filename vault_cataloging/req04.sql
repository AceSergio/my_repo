SELECT
CAST(AVG(COALESCE(NULLIF(rsa_time, 0), MAX(rsa_time) OVER())) AS DECIMAL(10, 2)) AS avg_rsa_time,
CAST(AVG(COALESCE(NULLIF(hyper_pulse_time, 0), MAX(hyper_pulse_time) OVER())) AS DECIMAL(10, 2)) AS avg_hyper_pulse_time,
CAST(AVG(COALESCE(NULLIF(quantum_x_time, 0), MAX(quantum_x_time) OVER())) AS DECIMAL(10, 2)) AS avg_quantum_x_time,
CAST(AVG(COALESCE(NULLIF(aes_time, 0), MAX(aes_time) OVER())) AS DECIMAL(10, 2)) AS avg_aes_time,
CAST(AVG(COALESCE(NULLIF(d_crypt_time, 0), MAX(d_crypt_time) OVER())) AS DECIMAL(10, 2)) AS avg_d_crypt_time
FROM dtf.madelines_files_results
LIMIT 1;

