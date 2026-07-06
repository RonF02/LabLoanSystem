# model 模块使用说明

## 文件

- `include/model.h`

## 作用

`model` 模块定义了系统的核心数据结构，用于描述物品、借用记录和归还记录。所有业务逻辑和存储接口均基于这些结构进行数据操作。

## 数据结构说明

### `Item`

表示实验室物品信息。

字段：

- `id`：唯一整数 ID
- `code`：物品编号（字符串）
- `name`：物品名称
- `model`：物品型号
- `quantity`：库存数量
- `description`：物品描述

使用场景：物品管理、借用库存检查、报表展示等。

### `BorrowRecord`

表示借用记录。

字段：

- `id`：借用记录 ID
- `user`：借用人姓名
- `item_code`：所借物品编号
- `quantity`：借用数量
- `borrow_date`：借用日期（YYYY-MM-DD）
- `due_date`：预计归还日期（YYYY-MM-DD）
- `status`：借用状态（`BORROW_ACTIVE` 或 `BORROW_RETURNED`）

使用场景：借用登记、借用查询、归还处理、报表统计。

### `ReturnRecord`

表示归还记录。

字段：

- `id`：归还记录 ID
- `borrow_id`：关联借用记录 ID
- `return_date`：归还日期（YYYY-MM-DD）
- `quantity`：归还数量

使用场景：归还登记、库存恢复、借用状态验证。

## 约束与建议

- `code` 与 `item_code` 必须匹配，且在业务逻辑中应作为关联字段使用。
- `status` 用于指示借用是否已归还；`BORROW_ACTIVE` 表示仍在借出中。
- `quantity` 应当为非负整数。
- 所有日期字符串应使用 `YYYY-MM-DD` 格式。
