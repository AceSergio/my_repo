SELECT DISTINCT
public.people.id,
public.people.first_name,
public.people.last_name
FROM public.people
JOIN transport.metro_usage_logs ON
transport.metro_usage_logs.person_id = public.people.id
JOIN transport.metro_stations ON
transport.metro_stations.id = transport.metro_usage_logs.station_id

WHERE EXISTS (
    SELECT 1 FROM transport.metro_usage_logs
    WHERE transport.metro_usage_logs.validation = 'ENTER'
)
AND transport.metro_stations.name = 'Morgane Abeille';
