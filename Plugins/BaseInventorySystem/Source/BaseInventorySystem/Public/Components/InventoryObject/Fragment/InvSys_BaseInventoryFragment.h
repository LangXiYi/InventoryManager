// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "InvSys_BaseInventoryFragment.generated.h"

class UInvSys_InventoryComponent;
/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BASEINVENTORYSYSTEM_API UInvSys_BaseInventoryFragment : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_PreEditInventoryObject;
	friend class UInvSys_BaseInventoryObject;

	#define COPY_INVENTORY_FRAGMENT_PROPERTY(c, v)\
		v = static_cast<c*>(PreEditFragment)->v;

public:
	UInvSys_BaseInventoryFragment();
	
	// [Client & Server]
	virtual void InitInventoryFragment(UObject* PreEditFragment);

	UFUNCTION(BlueprintCallable)
	virtual void RefreshInventoryFragment();

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE AActor* GetOwner() const;

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	FORCEINLINE FGameplayTag GetInventoryObjectTag() const;

	FORCEINLINE UInvSys_BaseInventoryObject* GetInventoryObject() const;

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool operator<(const UInvSys_BaseInventoryFragment& Other)
	{
		return this->Priority < Other.Priority;
	}

protected:
	// 该属性需要等待 RepNotify 通知完成后才能初始化，故在某些阶段可能为空！！
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_BaseInventoryObject> InventoryObject;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory Fragment")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY()
	AActor* Owner_Private;

	// UPROPERTY(EditDefaultsOnly, Category = "Inventory Fragment", meta = (ClampMin = 0))
	int32 Priority = INT_MAX;	// 执行 OnRefreshInventoryObject 时的优先级，数值越小的对象执行优先级越高
};