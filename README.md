# license_robot
license_robot主要用来对软件做授权. 支持

- 模块化授权
- 层次化授权：采用路径来记录各个层级,子路径会继续父路径的授权
- license文件名为机器序列号: 序列号根据机器硬件生成.防止license滥用
- license文件内容sha1加密，并生成摘要：可防止篡改
