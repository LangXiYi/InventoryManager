// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryItemInstance.h"
#include "GridInvSys_FoodInstance.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_FoodInstance : public UGridInvSys_InventoryItemInstance
{
	GENERATED_BODY()

public:
	UGridInvSys_FoodInstance();

	virtual void PostInitProperties() override;

	virtual void PreUpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 DeltaStackCount) override;

	UFUNCTION(BlueprintCallable)
	void TestFunction(float Time);

	virtual void InitInventoryItemInstance() override;

	virtual void BeginDestroy() override;

	virtual void NativeOnUseItemInstance() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 保鲜时间 [0 ~ 9999]，FreshTime <= 0 表示物品不可食用，
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance|Food", meta = (ExposeOnSpawn))
	float FreshTime = 10.f;

private:
	FTimerHandle FreshTimerHandle;
};
