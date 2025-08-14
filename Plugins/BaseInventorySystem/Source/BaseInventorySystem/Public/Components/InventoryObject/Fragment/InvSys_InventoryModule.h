// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryModule.generated.h"

class UInvSys_InventoryComponent;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryModule : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_BaseInventoryObject;

	#define COPY_INVENTORY_FRAGMENT_PROPERTY(c, v)\
		v = static_cast<c*>(PreEditFragment)->v;

public:
	UInvSys_InventoryModule();
	
	/** 初始化库存片段，库存组件:OnRep ---> 库存对象 ---> this */
	virtual void InitInventoryFragment(UObject* PreEditFragment) {}

	/** 刷新库存片段 */
	virtual void RefreshInventoryFragment() {}

	/** 模块内数据修改后必须调用该函数标记 */
	virtual void MarkInventoryModuleDirty();

public:
	/**
	 * Getter or Setter
	 */

	bool HasAuthority() const;

	ENetMode GetNetMode() const;

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	template<class T>
	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const
	{
		return (T*)InventoryComponent;
	}

	FORCEINLINE FGameplayTag GetInventoryTag() const
	{
		return InventoryTag;
	}

	FORCEINLINE UInvSys_BaseInventoryObject* GetInventoryObject() const
	{
		return InventoryObject;
	}

	FORCEINLINE AActor* GetOwner() const
	{
		return Owner_Private;
	}

	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }

	FORCEINLINE bool operator<(const UInvSys_InventoryModule& Other) const
	{
		return this->Priority < Other.Priority;
	}

	virtual bool ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	int32 InventoryModuleID = INDEX_NONE;
	int32 InventoryModuleRepKey = INDEX_NONE;

protected:
	// 客户端该属性需要等待 InitInventoryObject 执行才能初始化
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_BaseInventoryObject> InventoryObject;

	// 客户端该属性需要等待 InitInventoryObject 执行才能初始化
	// 客户端同步装备时会广播一次，该广播会获取当前的标签，但此时该属性为 None
	// UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory Fragment")
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	FGameplayTag InventoryTag;

	// UPROPERTY(EditDefaultsOnly, Category = "Inventory Fragment", meta = (ClampMin = 0))
	int32 Priority;	// 执行 OnRefreshInventoryObject 时的优先级，数值越小的对象执行优先级越高

	// Channel wants to go dormant (it will check during tick if it can go dormant)
	bool bPendingDormancy = true;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment", meta = (AllowPrivateAccess))
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment", meta = (AllowPrivateAccess))
	AActor* Owner_Private;
};