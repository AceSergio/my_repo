SELECT 
pb.id,
pb.street_id,
pb.created_at,
pb.person_id

FROM public.street_logs pb
WHERE pb.id IN (
    SELECT bk.id FROM backup.street_logs bk
);
