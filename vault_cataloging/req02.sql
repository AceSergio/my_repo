SELECT
id,
size AS stored_size,
decrypted,
CAST(
COALESCE
(
size,
CASE
WHEN decrypted = true THEN (SELECT AVG(size) FROM dtf.madelines_files_results WHERE parent_id IS NOT NULL AND decrypted = true)
ELSE (SELECT AVG(size) FROM dtf.madelines_files_results WHERE parent_id IS NOT NULL AND decrypted = false)
END
) 
AS bigint
) 
AS calculated_size
FROM dtf.madelines_files_results
WHERE parent_id IS NOT NULL
ORDER BY id ASC;
