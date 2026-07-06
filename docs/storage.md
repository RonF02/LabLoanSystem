# storage 模块使用说明

## 文件

- `include/storage.h`
- `src/storage.c`

## 作用

`storage` 模块负责系统的数据持久化，提供文件读写接口，支持将内存中的 `Item`、`BorrowRecord` 和 `ReturnRecord` 数据保存到 `data/` 目录下的 TXT 文件中，并从文件加载数据。

## 数据文件路径

- `data/items.txt`
- `data/borrow_records.txt`
- `data/return_records.txt`

模块会在启动时检查 `data/` 目录及以上文件是否存在，不存在时会尝试创建。

## 提供的接口

### `bool ensure_data_directory(void)`

- 作用：确保 `data/` 目录存在，并创建需要的 TXT 数据文件。
- 返回值：如果目录和文件创建成功，返回 `true`；否则返回 `false`。

### `bool load_items(Item items[], int *count)`

- 作用：从 `data/items.txt` 读取物品记录，解析为 `Item` 数组。
- 参数：
  - `items`：接收读取结果的数组。
  - `count`：返回读取到的记录数量。
- 返回值：读取成功返回 `true`，失败返回 `false`。

### `bool save_items(const Item items[], int count)`

- 作用：将 `Item` 数组写入 `data/items.txt`，覆盖原文件内容。
- 参数：
  - `items`：待保存的数组。
  - `count`：数组长度。
- 返回值：保存成功返回 `true`，失败返回 `false`。

### `bool load_borrow_records(BorrowRecord borrows[], int *count)`

- 作用：从 `data/borrow_records.txt` 读取借用记录。
- 参数与返回值同 `load_items`。

### `bool save_borrow_records(const BorrowRecord borrows[], int count)`

- 作用：将借用记录写入 `data/borrow_records.txt`。
- 参数与返回值同 `save_items`。

### `bool load_return_records(ReturnRecord returns[], int *count)`

- 作用：从 `data/return_records.txt` 读取归还记录。
- 参数与返回值同 `load_items`。

### `bool save_return_records(const ReturnRecord returns[], int count)`

- 作用：将归还记录写入 `data/return_records.txt`。
- 参数与返回值同 `save_items`。

## 文件格式说明

### `data/items.txt`

每行一条记录，字段用逗号分隔：

```
ID,code,name,model,quantity,description
```

### `data/borrow_records.txt`

每行一条记录，字段用逗号分隔：

```
ID,user,item_code,quantity,borrow_date,due_date,status
```

`status` 为整数，`0` 表示 `BORROW_ACTIVE`，`1` 表示 `BORROW_RETURNED`。

### `data/return_records.txt`

每行一条记录，字段用逗号分隔：

```
ID,borrow_id,return_date,quantity
```

## 注意事项

- 文件解析使用简单的逗号分隔规则，因此字段内容中不应包含逗号。
- 若文件读取失败，系统会以空数据继续运行，但应尽快检查文件路径与权限。
- 保存接口会覆盖目标文件，调用前请确保数据已完整准备。
