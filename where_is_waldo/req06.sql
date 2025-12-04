SELECT
nexus_stores.employees.id AS employee_id,
nexus_stores.employees.person_id,
people.first_name,
people.last_name
FROM nexus_stores.employees
JOIN people 
ON nexus_stores.employees.person_id = people.id
LEFT JOIN nexus_stores.cashier_shifts 
ON nexus_stores.employees.id = nexus_stores.cashier_shifts.employee_id 
AND nexus_stores.cashier_shifts.start_timestamp = '2059-12-01 13:00:00'
WHERE nexus_stores.employees.store_id = 78
AND nexus_stores.employees.position = 'CASHIER'
AND nexus_stores.cashier_shifts.id IS NULL;

