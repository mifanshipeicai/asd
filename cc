下面是基于 Ant Design 的实现方案，完成你的需求：

实现思路

	1.	使用 Ant Design 的 <Table> 组件来展示表格数据。
	2.	定义表格的 columns 和 dataSource。
	3.	在表格加载时，向后端接口发送请求，更新每行的状态。
	4.	动态修改状态列的数据。

示例代码

import React, { useState, useEffect } from "react";
import { Table, Tag, Button } from "antd";
import axios from "axios";

const Dashboard = () => {
  // 定义表格数据
  const [dataSource, setDataSource] = useState([
    { key: 1, apiName: "API 1", endpoint: "/api/endpoint1", status: "Pending" },
    { key: 2, apiName: "API 2", endpoint: "/api/endpoint2", status: "Pending" },
    { key: 3, apiName: "API 3", endpoint: "/api/endpoint3", status: "Pending" },
  ]);

  // 定义表格列
  const columns = [
    {
      title: "API Name",
      dataIndex: "apiName",
      key: "apiName",
    },
    {
      title: "Endpoint",
      dataIndex: "endpoint",
      key: "endpoint",
    },
    {
      title: "Status",
      dataIndex: "status",
      key: "status",
      render: (status) => {
        let color = "blue";
        if (status === "Success") color = "green";
        if (status === "Failed") color = "red";
        return <Tag color={color}>{status}</Tag>;
      },
    },
    {
      title: "Action",
      key: "action",
      render: (_, record) => (
        <Button type="primary" onClick={() => handleCheckStatus(record)}>
          Check Status
        </Button>
      ),
    },
  ];

  // 检查单个API的状态
  const handleCheckStatus = async (record) => {
    try {
      const response = await axios.get(record.endpoint);
      updateRowStatus(record.key, "Success");
    } catch (error) {
      updateRowStatus(record.key, "Failed");
    }
  };

  // 批量请求所有API的状态
  const fetchAllStatus = async () => {
    const updatedData = await Promise.all(
      dataSource.map(async (row) => {
        try {
          await axios.get(row.endpoint);
          return { ...row, status: "Success" };
        } catch (error) {
          return { ...row, status: "Failed" };
        }
      })
    );
    setDataSource(updatedData);
  };

  // 更新行状态
  const updateRowStatus = (key, status) => {
    setDataSource((prevData) =>
      prevData.map((row) =>
        row.key === key ? { ...row, status } : row
      )
    );
  };

  // 页面加载时请求所有状态
  useEffect(() => {
    fetchAllStatus();
  }, []);

  return (
    <div>
      <h1>API Status Dashboard</h1>
      <Table dataSource={dataSource} columns={columns} bordered />
    </div>
  );
};

export default Dashboard;

关键点说明

	1.	Ant Design 表格结构：
	•	dataSource 是表格的数据源。
	•	columns 定义表格的列，render 可以自定义单元格内容。
	2.	动态状态更新：
	•	handleCheckStatus 用于单个接口的状态更新。
	•	fetchAllStatus 用于批量更新所有接口状态。
	3.	Tag 的动态颜色：
	•	根据状态动态渲染不同颜色的标签。
	4.	异步接口请求：
	•	使用 axios 请求接口，接口调用成功则返回 Success，失败则返回 Failed。
	5.	重新渲染表格：
	•	使用 setDataSource 更新数据源，从而触发表格的重新渲染。

可选优化

	1.	轮询刷新：
如果需要定期检查状态，可以使用 setInterval 实现轮询更新。

useEffect(() => {
  const interval = setInterval(fetchAllStatus, 5000); // 每5秒刷新
  return () => clearInterval(interval);
}, []);


	2.	Loading 状态：
	•	可以为每个请求添加 loading 状态，使用 <Spin> 或 Button 的 loading 属性。
	3.	错误处理：
	•	提供用户可视化的错误信息，结合 Ant Design 的 message 或 notification 组件。

如果需要更多功能扩展或进一步优化，可以随时讨论！