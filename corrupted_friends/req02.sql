INSERT INTO dtf.madelines_contacts (id, title, first_name, last_name, phone, email, favorite, created_at)
SELECT
    id,
    CAST(split_part(INITCAP(TRIM(REGEXP_REPLACE(full_name, '[^a-zA-Z ]', '', 'g'))), ' ', 1) AS public.prefix),
    split_part(INITCAP(TRIM(REGEXP_REPLACE(full_name, '[^a-zA-Z ]', '', 'g'))), ' ', 2),
    split_part(INITCAP(TRIM(REGEXP_REPLACE(full_name, '[^a-zA-Z ]', '', 'g'))), ' ', 3),
    (
        LPAD(split_part(TRANSLATE(REGEXP_REPLACE(phone, '[^0-9.-]', '', 'g'), '-', '.'), '.', 1), 3, '0') || '.' ||
        LPAD(split_part(TRANSLATE(REGEXP_REPLACE(phone, '[^0-9.-]', '', 'g'), '-', '.'), '.', 2), 3, '0') || '.' ||
        LPAD(split_part(TRANSLATE(REGEXP_REPLACE(phone, '[^0-9.-]', '', 'g'), '-', '.'), '.', 3), 3, '0')
    ) AS phone,
    email,
    (favorite = '1'),
    CAST(created_at AS TIMESTAMP)
FROM dtf.madelines_contacts_corrupted;

