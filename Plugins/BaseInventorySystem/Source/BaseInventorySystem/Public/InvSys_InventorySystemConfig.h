// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InvSys_InventorySystemConfig.generated.h"

/**
 * GetDefault ---> CDO
 * 修改配置文件时，默认值也会自动修改，且重启编辑器后通过CDO（类默认值）的方式获取的值仍然是上次修改后的值。
 */
UCLASS(DefaultConfig, Config = "BaseInventorySystemConfig")
class BASEINVENTORYSYSTEM_API UInvSys_InventorySystemConfig : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// UDeveloperSettings Begin =====
	virtual FName GetContainerName() const override { return "Project"; }
	virtual FName GetCategoryName() const override { return "InventorySystem"; }
	virtual FName GetSectionName() const override { return "BaseInventorySystemConfig"; }
	// UDeveloperSettings Over =====
	
public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Inventory System Config")
	FName ConfigSettings;

	// 服务器等待合批处理的时间，该值越小则服务器发送给客户端的频率就越快。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Inventory System Config", meta=(ClampMin = 0.01663f))
	float ServerWaitBatchTime = 0.1f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Inventory System Config")
	FName TestName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Inventory System Config")
	FName Test;
};
