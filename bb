要在 React 前端工程中设计一个健康检查的 Dashboard，通过表格显示指定接口的调用状态，可以参考以下步骤：

功能需求

	1.	表格列内容：
	•	接口名称
	•	调用状态（成功/失败）
	•	最近调用时间
	•	响应时间
	•	错误信息（如果有）
	2.	实现逻辑：
	•	定期调用指定接口，检查状态。
	•	根据接口返回结果更新表格状态。
	3.	设计样式：
	•	使用组件库（如 Ant Design 或 Material-UI）来快速构建表格。
	•	状态列使用颜色标注（如绿色表示成功，红色表示失败）。

实现步骤

1. 创建健康检查数据结构

定义一个接口列表，每个接口的健康状态存储在状态管理中：

const initialApiStatus = [
  { name: "User API", url: "/api/users", status: "unknown", lastChecked: null, responseTime: null, errorMessage: null },
  { name: "Order API", url: "/api/orders", status: "unknown", lastChecked: null, responseTime: null, errorMessage: null },
  { name: "Product API", url: "/api/products", status: "unknown", lastChecked: null, responseTime: null, errorMessage: null },
];

2. 定期调用接口

使用 useEffect 和 setInterval 定期检查接口状态：

import React, { useState, useEffect } from "react";

function HealthCheckDashboard() {
  const [apiStatus, setApiStatus] = useState(initialApiStatus);

  useEffect(() => {
    const interval = setInterval(() => {
      checkApis();
    }, 60000); // 每分钟检查一次
    return () => clearInterval(interval); // 清除定时器
  }, []);

  const checkApis = async () => {
    const updatedStatus = await Promise.all(
      apiStatus.map(async (api) => {
        const startTime = Date.now();
        try {
          const response = await fetch(api.url);
          const responseTime = Date.now() - startTime;
          return {
            ...api,
            status: response.ok ? "success" : "failed",
            lastChecked: new Date().toLocaleString(),
            responseTime: `${responseTime}ms`,
            errorMessage: response.ok ? null : `Error: ${response.status}`,
          };
        } catch (error) {
          return {
            ...api,
            status: "failed",
            lastChecked: new Date().toLocaleString(),
            responseTime: null,
            errorMessage: error.message,
          };
        }
      })
    );
    setApiStatus(updatedStatus);
  };

  return (
    <div>
      <h1>API Health Check Dashboard</h1>
      <HealthCheckTable apiStatus={apiStatus} />
    </div>
  );
}

3. 表格组件

使用表格显示接口状态，可以通过 Ant Design 或手写表格：

使用 Ant Design 表格

import { Table, Tag } from "antd";

function HealthCheckTable({ apiStatus }) {
  const columns = [
    { title: "API Name", dataIndex: "name", key: "name" },
    {
      title: "Status",
      dataIndex: "status",
      key: "status",
      render: (status) => (
        <Tag color={status === "success" ? "green" : "red"}>
          {status.toUpperCase()}
        </Tag>
      ),
    },
    { title: "Last Checked", dataIndex: "lastChecked", key: "lastChecked" },
    { title: "Response Time", dataIndex: "responseTime", key: "responseTime" },
    {
      title: "Error Message",
      dataIndex: "errorMessage",
      key: "errorMessage",
      render: (errorMessage) => errorMessage || "None",
    },
  ];

  return <Table dataSource={apiStatus} columns={columns} rowKey="name" />;
}

手写表格 (不使用组件库)

function HealthCheckTable({ apiStatus }) {
  return (
    <table border="1" style={{ width: "100%", textAlign: "left" }}>
      <thead>
        <tr>
          <th>API Name</th>
          <th>Status</th>
          <th>Last Checked</th>
          <th>Response Time</th>
          <th>Error Message</th>
        </tr>
      </thead>
      <tbody>
        {apiStatus.map((api) => (
          <tr key={api.name}>
            <td>{api.name}</td>
            <td style={{ color: api.status === "success" ? "green" : "red" }}>
              {api.status.toUpperCase()}
            </td>
            <td>{api.lastChecked || "Not Checked Yet"}</td>
            <td>{api.responseTime || "N/A"}</td>
            <td>{api.errorMessage || "None"}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}

4. 样式优化

结合 CSS 或 Ant Design 内置的样式，可以让表格更直观，状态用颜色和图标标示成功/失败。

运行结果

你将得到一个可以实时显示 API 健康状况的 Dashboard 表格，包括以下信息：
	•	接口名称。
	•	当前状态（成功/失败）。
	•	最近检查时间。
	•	响应时间。
	•	错误信息（若有）。

是否需要进一步优化或添加额外功能？