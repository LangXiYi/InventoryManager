
#pragma once

// #include "InvSys_CommonType.generated.h"


// 持久化保存结构
/**
 * UniqueId
 * ItemID
 * SlotName
 * 以下具体结构可能会根据需要发生改变。
 * GridId
 * Position
 * Direction
 */
// Server 读取数据后，生成实例，并将数据发送给玩家
// 发送的数据结构？
/*
 * 客户端接收的结构应该是如何的？大致与服务器应该是一致的，使用Object同步数据？Object管理物品实例。
 * 一个Object就代表一个物品？貌似可行
 * UI与Object如何关联？
 * Object内部存在位置信息，所以Object本身是知道它的位置的，通过Object获取关联的UI即可。
 * 
 * 
 * UniqueID
 * ItemID（X） ===> ItemInfo
 * SlotName
 * 以下具体结构可能会根据需要发生改变。
 * GridId
 * Position
 * Direction
 *
 * 比如这个物品可以被装备，右键会有可装备的选项
 * 比如这个物品可以被拆分
 * 可以被丢弃
 * 可以被使用
 * 可以被旋转？
 * 等等
 * 使用Fragment，一个TArray
 * 表明该物品的用途
 * 右键物品时，会根据ItemInfo遍历目标的Fragment，然后由Fragment创建自身的右键按钮，最后一起呈现
 *		点击时，触发Fragment内部的函数。
 * Fragment应该不是复制的，而是在客户端接收到ItemID后，根据 DataAsset 自己创建的？
 * 如果是使用类型的Fragment，是否意味着需要发送RPC？
 *		可以通过InventoryComp，发送使用请求
 * 如果是装备、丢弃等应该也都是通过InvComp进行中转。
 * 他们应该是存储的DataAsset中还是其他？
 * 
 * */