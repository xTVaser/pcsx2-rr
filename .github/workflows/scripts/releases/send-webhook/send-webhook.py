from discord import Embed, Webhook, AsyncWebhookAdapter
import aiohttp
import asyncio
import os

async def send_webhook():
    async with aiohttp.ClientSession() as session:
        webhook = Webhook.from_url(
            os.environ["DISCORD_BUILD_WEBHOOK"], adapter=AsyncWebhookAdapter(session)
        )

        # TODO - get the asset links

        embed = Embed(
            title="New PCSX2 Nightly Build Available!", color=0xFF8000
        )
        embed.add_field(name="Version", value="v1.7.9999", inline=True)
        embed.add_field(name="Release Link", value="https://www.google.com", inline=True)
        embed.add_field(name="Installation Steps", value="www.google.com", inline=True)
        embed.add_field(
            name="Windows Downloads",
            value="- Something\n- something else",
            inline=False,
        )
        embed.add_field(name="Linux Downloads", value="- Something", inline=True)

        await webhook.send(embed=embed)

asyncio.get_event_loop().run_until_complete(send_webhook())
