// Fill out your copyright notice in the Description page of Project Settings.


#include "TestActor.h"
#include "TestObjectEntry.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATestActor::ATestActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicateUsingRegisteredSubObjectList = false;
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		
		UTestObjectEntry* TestObjectEntryA = NewObject<UTestObjectEntry>(this);
		TestObjectEntryA->Name = "TestObjectEntryA";
		MyStruct.Entries.Add(TestObjectEntryA);
	
		UTestObjectEntry* TestObjectEntryB = NewObject<UTestObjectEntry>(this);
		TestObjectEntryB->Name = "TestObjectEntryB";
		MyStruct.Entries.Add(TestObjectEntryB);

		TestObjectEntryC = NewObject<UTestObjectEntry>(this);
		TestObjectEntryC->Name = "TestObjectEntryC";
		MyStruct.Entries.Add(TestObjectEntryC);

		FTimerHandle TempTimer;
		GetWorld()->GetTimerManager().SetTimer(TempTimer, [&]()
		{
			MyStruct.Entries.Remove(TestObjectEntryC); // 移除掉最后一个元素
		}, 2.f, false);

		FTimerHandle TempTimerB;
		GetWorld()->GetTimerManager().SetTimer(TempTimerB, [&]()
		{
			MyStruct.Entries[0]->Name = "Update_A";
			MyStruct.Entries[1]->Name = "Update_B";
			
			TestObjectEntryC->Name = "Update_C";
			MyStruct.Entries.Add(TestObjectEntryC);
			TestObjectEntryC = nullptr;
		}, 5.f, false);
	}
}

void ATestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATestActor, MyStruct, COND_None);
}

bool ATestActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bIsUpdate = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UTestObjectEntry* Entry : MyStruct.Entries)
	{
		if (Entry)
		{
			bIsUpdate |= Channel->ReplicateSubobject(Entry, *Bunch, *RepFlags);
			UE_CLOG(bIsUpdate, LogTemp, Error, TEXT("Replicated Object ---> %s"), *Entry->Name)
		}
	}
	if (TestObjectEntryC)
	{
		//bIsUpdate |= Channel->ReplicateSubobject(TestObjectEntryC, *Bunch, *RepFlags);
		UE_CLOG(bIsUpdate, LogTemp, Error, TEXT("TTT Replicated Object ---> %s"), *TestObjectEntryC->Name)
	}
	return bIsUpdate;
}
