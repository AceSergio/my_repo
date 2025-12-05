DELETE FROM public.epix_posts
USING public.epix_hashtags, public.epix_accounts, public.people
WHERE epix_posts.hashtag_id = epix_hashtags.id
AND epix_posts.author_id = epix_accounts.id
AND epix_accounts.person_id = public.people.id
AND epix_hashtags.name = 'EndSurveillance'
RETURNING public.people.first_name, public.people.last_name, public.epix_accounts.username, public.epix_posts.body;


