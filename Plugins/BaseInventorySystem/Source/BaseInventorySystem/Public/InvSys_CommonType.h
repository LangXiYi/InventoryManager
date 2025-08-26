
#pragma once

// TODO:: 
UENUM(BlueprintType)
enum class EInvSys_InventoryWidgetActivity : uint8
{
	None = 0,
	// 持久的
	Permanent,
	// 临时的
	Interim,
	// 自定义
	Custom,
};