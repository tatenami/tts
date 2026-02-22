# TTS
C++ の coroutine (コルーチン) を用いた協調型タスクスケジューラ．
コルーチン関数をタスクの単位としてスケジューリングする．

## タスクの作成と使用例
 - タスクの定義
   - 戻り値型が `Task` の関数であること
   - 関数内に `co_await` + スケジューリング関連のAPI(後述) の記述があること
 - タスクの作成
   - 上記のコルーチン関数，名前を引数に `create_task` を使用する

プログラム例
```cpp
Task task1() {
  for (int i = 1; i <= 3; i++) { 
    printf("[task1] num: %d\n", i);
    co_await sleep_ms(5);
  }
}

Task task2() {
  for (int i = 1; i <= 3; i++) {
    printf("[task2] num: %d\n", i);
    co_await sleep_ms(2);
  }
}

int main() {
  create_task("task1", task1);
  create_task("task2", task2);

  start_scheduler();

  return 0;
}
```
出力
```bash
[task1] num: 1
[task2] num: 1
[task2] num: 2
[task2] num: 3
[task1] num: 2
[task1] num: 3
```

## スケジューリングAPI
スケジューリングのため，タスクは処理内で自ら処理を手放す必要がある．
その契機が `co_await` と，それとセットで呼び出すスケジューリングAPIである．

 - `yield()`
 - `suspend()`
 - `sleep_ms(ms)`
  
スケジューリング用問わず，タスク制御に関連するAPIは順次追加する