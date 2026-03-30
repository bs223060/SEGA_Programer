#include "Enemy_Zako.h"
#include "Kismet/GameplayStatics.h"
#include "MyProject/Core/MyGameInstance.h" 

AEnemy_Zako::AEnemy_Zako()
{
	PrimaryActorTick.bCanEverTick = false;

	BattleTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("BattleTrigger"));
	BattleTrigger->SetupAttachment(RootComponent);
	BattleTrigger->SetSphereRadius(250.0f);

	BattleTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BattleTrigger->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BattleTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BattleTrigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	BattleTrigger->SetGenerateOverlapEvents(true);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	MaxHP = 100.0f;
}

void AEnemy_Zako::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;

	if (BattleTrigger)
	{
		BattleTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEnemy_Zako::OnOverlapBegin);
	}
}

float AEnemy_Zako::GetHealthPercentage() const
{
	return (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
}

void AEnemy_Zako::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (GI)
		{
			GI->DefeatedEnemyNames.AddUnique(this->EnemyUniqueName);

			GI->CurrentEnemyID = this->EnemyID;
			GI->IsBossBattle = false;
			GI->PlayerPreBattleTransform = OtherActor->GetActorTransform();
			GI->EnemyClassToSpawn = this->GetClass();

			UGameplayStatics::OpenLevel(this, FName("L_Battle"));
		}
	}
}


void AEnemy_Zako::TakeZakoDamage(float DamageAmount)
{
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->bIsReturningFromBattle = true;
			GI->DefeatedEnemyNames.AddUnique(this->GetFName());
		}

		ABattleCharacter* Player = Cast<ABattleCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (Player)
		{
			Player->GainExp(15);
		}

		Destroy();
	}
}