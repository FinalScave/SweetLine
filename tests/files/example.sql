-- SQL 高亮示例

-- 创建表
CREATE TABLE users (
    id          SERIAL PRIMARY KEY,
    name        VARCHAR(100) NOT NULL,
    email       VARCHAR(255) UNIQUE,
    age         INT DEFAULT 0,
    salary      DECIMAL(10, 2),
    is_active   BOOLEAN DEFAULT TRUE,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_users_email ON users(email);

-- 插入数据
INSERT INTO users (name, email, age, salary, is_active)
VALUES ('Alice', 'alice@example.com', 30, 75000.50, TRUE),
       ('Bob', 'bob@example.com', 25, 60000.00, FALSE);

-- 基本查询
SELECT u.id, u.name, u.email
FROM users u
WHERE u.is_active = TRUE
  AND u.age BETWEEN 20 AND 40
  AND u.name LIKE 'A%'
  AND u.email IS NOT NULL
ORDER BY u.name ASC
LIMIT 10 OFFSET 0;

-- 聚合函数 (builtin)
SELECT COUNT(*) AS total,
       AVG(salary) AS avg_salary,
       MAX(salary) AS max_salary,
       MIN(salary) AS min_salary,
       SUM(salary) AS total_salary,
       ROUND(AVG(age), 2) AS avg_age
FROM users
WHERE is_active = TRUE
GROUP BY is_active
HAVING COUNT(*) > 1;

-- 窗口函数 (builtin)
SELECT name,
       salary,
       ROW_NUMBER() OVER (ORDER BY salary DESC) AS rank,
       DENSE_RANK() OVER (ORDER BY salary DESC) AS dense_rank,
       LAG(salary) OVER (ORDER BY salary) AS prev_salary,
       LEAD(salary) OVER (ORDER BY salary) AS next_salary
FROM users;

-- 字符串函数 (builtin)
SELECT UPPER(name) AS upper_name,
       LOWER(email) AS lower_email,
       LENGTH(name) AS name_len,
       SUBSTRING(name, 1, 3) AS short_name,
       CONCAT(name, ' <', email, '>') AS full_info,
       TRIM(name) AS trimmed,
       REPLACE(email, '@', ' at ') AS masked_email,
       COALESCE(email, 'N/A') AS safe_email
FROM users;

-- 日期函数 (builtin)
SELECT NOW() AS current_time,
       EXTRACT(YEAR FROM created_at) AS year,
       DATE_TRUNC('month', created_at) AS month_start
FROM users;

-- JSON 函数 (builtin)
SELECT JSON_OBJECT('name', name, 'age', age) AS user_json,
       JSON_ARRAY(name, email) AS user_array
FROM users;

-- JOIN 查询
SELECT u.name, o.amount
FROM users u
INNER JOIN orders o ON u.id = o.user_id
LEFT JOIN payments p ON o.id = p.order_id
WHERE o.amount > 100.00
  AND p.id IS NULL;

-- 子查询和 CTE
WITH active_users AS (
    SELECT id, name, salary
    FROM users
    WHERE is_active = TRUE
)
SELECT name, salary
FROM active_users
WHERE salary > (SELECT AVG(salary) FROM active_users);

-- CASE 表达式
SELECT name,
       CASE
           WHEN age < 18 THEN 'minor'
           WHEN age < 65 THEN 'adult'
           ELSE 'senior'
       END AS category,
       NULLIF(salary, 0) AS safe_salary,
       CAST(age AS VARCHAR) AS age_str
FROM users;

-- 更新和删除
UPDATE users SET salary = salary * 1.1 WHERE is_active = TRUE;
DELETE FROM users WHERE is_active = FALSE AND age < 18;

-- 事务
BEGIN TRANSACTION;
    UPDATE users SET salary = 80000 WHERE name = 'Alice';
    COMMIT;

/* 多行注释
   跨越多行 */
