SELECT
child.id,
child.filename,
CASE
WHEN child.decrypted = true THEN 'File was successfully decrypted.'
WHEN child.decrypted = false AND parent.decrypted = true THEN 'File was successfully decrypted because its containing folder was successfully decrypted.'
ELSE 'File remains encrypted.'
END 
AS decryption_status
FROM dtf.madelines_files_results AS child
LEFT JOIN dtf.madelines_files_results AS parent 
ON child.parent_id = parent.id
ORDER BY child.id ASC;

