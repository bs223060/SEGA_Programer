#include "MyBossEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/MyGameInstance.h"

AMyBossEnemy::AMyBossEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    BattleTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BattleTrigger"));
    BattleTrigger->SetupAttachment(RootComponent);
    BattleTrigger->SetBoxExtent(FVector(200.0f, 2000.0f, 500.0f));

    BattleTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BattleTrigger->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    BattleTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    BattleTrigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    BattleTrigger->SetGenerateOverlapEvents(true);
}

void AMyBossEnemy::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;

    if (BattleTrigger)
    {
        BattleTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMyBossEnemy::OnOverlapBegin);
    }
}

void AMyBossEnemy::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor->ActorHasTag("Player"))
    {
        UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
        if (GI)
        {
            GI->IsBossBattle = true;
            GI->PlayerPreBattleTransform = OtherActor->GetActorTransform();

            GI->EnemyClassToSpawn = this->GetClass();

            UGameplayStatics::OpenLevel(this, FName("L_Battle"));
        }
    }
}

void AMyBossEnemy::TakeBossDamage(float DamageAmount)
{
    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

    if (CurrentHP <= 0.0f)
    {
        ABattleCharacter* Player = Cast<ABattleCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (Player)
        {
            Player->GainExp(60);
        }

        UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
        if (GI)
        {
            GI->bIsBossDead = true;
            GI->bIsReturningFromBattle = true;
            GI->DefeatedEnemyNames.AddUnique(this->GetFName());
        }
        UGameplayStatics::OpenLevel(this, FName("Map_Start"));
    }
}

float AMyBossEnemy::GetHealthPercentage() const
{
    return (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
}

EBossAttackType AMyBossEnemy::GetNextAttackType()
{
    return (GetHealthPercentage() <= 0.5f) ? EBossAttackType::Heavy : EBossAttackType::Normal;
}

float AMyBossEnemy::CalculateAttackDamage()
{
    float BaseDamage = bIsCharging ? 50.0f : 20.0f;
    return (GetHealthPercentage() <= 0.3f) ? BaseDamage * 1.5f : BaseDamage;
}

void AMyBossEnemy::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
void AMyBossEnemy::CheckHealthStatus() {  }
void AMyBossEnemy::ExecuteSpecialAttack() {  }