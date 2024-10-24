# Setup environment **只測試程式可以跳到Test那節**
```
python3 setup_state.py [file_segment]
```

確定file_segment的個數，由setup_state.py建立state資料夾，並在裡面建立對應數量(NUMFD = 2^file_segment)的state檔。

# Circuit Format

check README.md in circuit folder.


# Test **可以獨立執行**

```
python3 test.py
```
可以加參數
--N [Number of total bit]
--NGQB [Number of global Qubit]
--NSQB [Number of thread Qubit]
--NLQB [Number of local Qubit]
default 12 3 6 3

裡面會自動執行全部的test

在每一個test裡面會利用for迴圈進行下面的工作
1. 建立circuit
2. 用我們的模擬器執行這份circuit
3. 利用Qiskit執行同樣的circuit
4. 檢驗是否在1e-9的標準下跟Qiskit的結果吻合。 (使用float儲存狀態時，建議降低標準到1e-3)

# ini/circuit/res
sdmg: single disk multi gate
mdmg: multi disk multi gate
