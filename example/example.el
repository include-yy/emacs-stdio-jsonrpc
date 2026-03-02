(require 'jsonrpc)

;; Change to example.exe's path if needed.
(setq path (expand-file-name "../out/build/x64-Debug/example.exe"))

(setq rpc (let ((proc (make-process :name "example"
                                    :command `(,path)
                                    ;; binary is necessary on Windows
                                    :coding 'binary) ))
            (make-instance 'jsonrpc-process-connection
                           :name "jsonrpc-example"
                           :process proc)))

(jsonrpc-request rpc "add" [1 2])
;;=> 3.0
(jsonrpc-request rpc "heavy_task" nil)
;;=> Wait 3 seconds
(jsonrpc-notify rpc "exit" nil)
;;=> exit subprocess
