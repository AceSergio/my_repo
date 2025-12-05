SELECT 
bk.id,
bk.street_id,
bk.created_at,
bk.person_id
FROM backup.street_logs bk

WHERE bk.id NOT IN (
    SELECT id FROM public.street_logs
);

