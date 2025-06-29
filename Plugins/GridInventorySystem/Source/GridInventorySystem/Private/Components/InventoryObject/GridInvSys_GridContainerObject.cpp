// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/GridInvSys_GridContainerObject.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Net/UnrealNetwork.h"

void UGridInvSys_GridContainerObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
	UObject* PreEditPayLoad)
{
	Super::InitInventoryObject(NewInventoryComponent, PreEditPayLoad);


}

void UGridInvSys_GridContainerObject::AddInventoryItemToContainer(const FGridInvSys_InventoryItem& NewItem)
{
	// ContainerItems.Add(InventoryItem);
}

void UGridInvSys_GridContainerObject::CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent,
                                                              UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(NewInventoryComponent, PreEditPayLoad);
	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridContainerObject, ContainerGridLayoutWidgetClass)
}

void UGridInvSys_GridContainerObject::CreateDisplayWidget(APlayerController* PC)
{
	Super::CreateDisplayWidget(PC);
	// 创建网格容器的布局控件
	if (PC && PC->IsLocalController())
	{
#if WITH_EDITOR
		if (InventoryComponent->IsA(UGridInvSys_InventoryComponent::StaticClass()) == false)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("库存组件类型与填充的 InventoryObject 的类型不匹配。"))
			return;
		}
#endif
		UGridInvSys_InventoryComponent* GridInventoryComponent = static_cast<UGridInvSys_InventoryComponent*>(InventoryComponent);

		ContainerGridLayoutWidget = CreateWidget(PC, ContainerGridLayoutWidgetClass);

		// 将网格容器的布局控件添加到命名插槽的位置
		UUserWidget* InventoryMenuWidget = GridInventoryComponent->GetInventoryLayoutWidget();
		if (InventoryMenuWidget == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("InventoryMenuWidget GridInventoryComponent 中不存在"));
			return;
		}
		UNamedSlot* NamedSlot = Cast<UNamedSlot>(InventoryMenuWidget->GetWidgetFromName(GetSlotName()));
#if WITH_EDITOR
		if (NamedSlot == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("命名槽[%s]在 InventoryMenuWidget 中不存在"), *GetSlotName().ToString())
			return;
		}
#endif
		NamedSlot->AddChild(ContainerGridLayoutWidget);
	}
}

void UGridInvSys_GridContainerObject::AddInventoryItemToContainer(FName ItemUniqueID, FGridInvSys_InventoryItem NewItem)
{
	// InventoryItems.Add(NewItem);
	// Server_XXX->Add(Key, Value);
}

void UGridInvSys_GridContainerObject::RemoveInventoryItemFromContainer(FName ItemUniqueID)
{
	
}

void UGridInvSys_GridContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGridInvSys_GridContainerObject, InventoryItems, COND_None);
}

void UGridInvSys_GridContainerObject::OnRep_InventoryItems()
{
	
}