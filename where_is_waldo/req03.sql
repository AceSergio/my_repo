SELECT
COUNT(*) - COUNT(store_id) AS store_id_null_count,
COUNT(*) - COUNT(employee_id) AS employee_id_null_count
FROM nexus_stores.cashier_shifts;
