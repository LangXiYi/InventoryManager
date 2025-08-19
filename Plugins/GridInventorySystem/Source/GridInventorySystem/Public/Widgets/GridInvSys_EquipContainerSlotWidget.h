// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "GridInvSys_EquipmentSlotWidget.h"
#include "Components/NamedSlot.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GridInvSys_EquipContainerSlotWidget.generated.h"

struct FGridInvSys_ItemPosition;
class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridItemWidget;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipContainerSlotWidget : public UGridInvSys_EquipmentSlotWidget
{
	GENERATED_BODY()

public:
	virtual void InitInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject) override;

	virtual void EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance) override;

	virtual void UnEquipItemInstance() override;

	template<class T>
	T* GetContainerLayoutWidget() const
	{
		if (NS_ContainerGridLayout->HasAnyChildren())
		{
			UWidget* ChildAt = NS_ContainerGridLayout->GetChildAt(0);
			if (ChildAt->IsA<T>())
			{
				return (T*)NS_ContainerGridLayout->GetChildAt(0);
			}
			checkNoEntry()
		}
		else
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("未在 NS_ContainerGridLayout 下发现任何控件，容器布局必须添加在 NS_ContainerGridLayout 下。"))
		}
		return nullptr;
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UNamedSlot> NS_ContainerGridLayout;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment Container Slot Widget")
	UGridInvSys_ContainerGridLayoutWidget* ContainerLayout;
	
private:
	FGameplayMessageListenerHandle OnAddItemInstanceHandle;
	FGameplayMessageListenerHandle OnRemoveItemInstanceHandle;

};
