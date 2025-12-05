UPDATE public.epix_posts
SET downvotes = CAST(
    upvotes * (
        SELECT CAST(SUM(p.downvotes) AS FLOAT) / SUM(p.upvotes)
        FROM public.epix_posts p
        JOIN public.epix_accounts a ON p.author_id = a.id
        JOIN public.people pe ON a.person_id = pe.id
        WHERE pe.first_name = 'Amina' 
        AND pe.last_name = 'Dubois'
        AND p.id != 139
    ) AS INTEGER
)
WHERE id = 139;

