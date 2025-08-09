// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "InvSys_BaseInventoryFragment.generated.h"

class UInvSys_InventoryComponent;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BASEINVENTORYSYSTEM_API UInvSys_BaseInventoryFragment : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_BaseInventoryObject;

	#define COPY_INVENTORY_FRAGMENT_PROPERTY(c, v)\
		v = static_cast<c*>(PreEditFragment)->v;

public:
	UInvSys_BaseInventoryFragment();
	
	/** 初始化库存片段，库存组件:OnRep ---> 库存对象 ---> this */
	virtual void InitInventoryFragment(UObject* PreEditFragment) {}

	/** 刷新库存片段 */
	virtual void RefreshInventoryFragment() {}

public:
	/**
	 * Getter or Setter
	 */

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	template<class T>
	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const
	{
		return (T*)InventoryComponent;
	}

	FORCEINLINE FGameplayTag GetInventoryObjectTag() const;

	FORCEINLINE UInvSys_BaseInventoryObject* GetInventoryObject() const;

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE AActor* GetOwner() const;

	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }

	FORCEINLINE bool operator<(const UInvSys_BaseInventoryFragment& Other) const
	{
		return this->Priority < Other.Priority;
	}

	virtual bool ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 客户端该属性需要等待 InitInventoryObject 执行才能初始化
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_BaseInventoryObject> InventoryObject;

	// 客户端该属性需要等待 InitInventoryObject 执行才能初始化
	// 客户端同步装备时会广播一次，该广播会获取当前的标签，但此时该属性为 None
	// UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory Fragment")
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	FGameplayTag InventoryObjectTag;

	// UPROPERTY(EditDefaultsOnly, Category = "Inventory Fragment", meta = (ClampMin = 0))
	int32 Priority;	// 执行 OnRefreshInventoryObject 时的优先级，数值越小的对象执行优先级越高

private:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment", meta = (AllowPrivateAccess))
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment", meta = (AllowPrivateAccess))
	AActor* Owner_Private;
};