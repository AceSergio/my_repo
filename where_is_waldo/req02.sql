SELECT nexus_stores.cashier_shifts.id, nexus_stores.cashier_shifts.checkout_id,
nexus_stores.cashier_shifts.employee_id,
nexus_stores.cashier_shifts.store_id,
nexus_stores.cashier_shifts.start_timestamp,
nexus_stores.cashier_shifts.end_timestamp
FROM nexus_stores.cashier_shifts 
JOIN nexus_stores.employees 
ON nexus_stores.cashier_shifts.employee_id = nexus_stores.employees.id
JOIN nexus_stores.stores 
ON nexus_stores.cashier_shifts.store_id = nexus_stores.stores.id;


